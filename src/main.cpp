/*#include <iostream>
#include "MNN/ImageProcess.hpp"

using namespace std;

int main()
{
    cout << "Hello World!" << endl;
    return 0;
}*/

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>


using namespace std;
using namespace MNN;

#define hard_nms 1
#define blending_nms 2

const float mean_vals[3] = {0.f, 0.f, 0.f};
const float norm_vals[3] = {1/256.0f, 1/256.0f, 1/256.0f};
const float thres[2] = {0.6,0.5} ;
const int BIAS_W[6] = {9, 14, 20, 31, 41, 64};
const int BIAS_H[6] = {14, 19, 27, 31, 48, 80};
const int INPUT_SIZE = 256;
const int CLASS_NUM = 1;

struct BBox {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
};

float sigmod(float x){
    return 1.0 / (1.0 + exp(-x));
}

int topK(float * labels,int size){
    int index = 0;
    float maxid = labels[0];

    for(int i = 1;i < size;i++){

        if(labels[i]>maxid){
            index = i;
            maxid = labels[i];
        }
    }
    return index;
}



void postprocess(std::vector<MNN::Tensor*> output,std::vector<BBox> &boxes,int iw,int ih){

    //fpn 图像金字塔
    for(int fpn = 0;fpn < 2;fpn++){

        int channel = output[fpn]->channel();
        int width = output[fpn]->width();
        int fmscale = width*width;
        int fmsize = 5+CLASS_NUM;

        //dect_box_handle {"10":0.6, "20":0.5}
        //32倍下采样 fm_size = input_size/32
        for(int c = 0;c < fmscale;c++){
            int x = c % width;
            int y = c / width;
            // 3种anchor box
            for(int s = 0;s < 3;s++){
                float scores = sigmod( output[fpn]->host<float>()[ c * channel + s * fmsize + 4 ] );
                if(scores > thres[fpn]){

                //cout<<scores<<" "<<c<< " x: "<<x<<"   y: "<<y<<" anchor: "<<s<<endl;
                BBox rect;
                float xx = ((sigmod(output[fpn]->host<float>()[c * channel + s * fmsize ]) + x) / width) * iw;
                float yy = ((sigmod(output[fpn]->host<float>()[c * channel + s * fmsize +1]) + y) / width) * ih;
                float w = ((BIAS_W[fpn * 3 + s] * exp(output[fpn]->host<float>()[c * channel + s * fmsize + 2]))/INPUT_SIZE) * iw;
                float h = ((BIAS_H[fpn * 3 + s] * exp(output[fpn]->host<float>()[c * channel + s * fmsize + 3]))/INPUT_SIZE) * ih;
                rect.x1 = int(xx - w * 0.5);
                rect.x2 = int(xx + w * 0.5);
                rect.y1 = int(yy - h * 0.5);
                rect.y2 = int(yy + h * 0.5);
                rect.score = scores;
                rect.label = topK(output[fpn]->host<float>()+c * channel + s * fmsize +5,CLASS_NUM);
                boxes.push_back(rect);
                //cout<<rect.x1<< " "<<rect.y1<<" "<<rect.x2<<" "<<rect.y2<<endl;
                }
            }
        }
    }
}

void nms(std::vector<BBox> &input,std::vector<BBox> &output,float iou_threshold,int type){
    if(input.size()!=0){
        std::sort(input.begin(), input.end(), [](const BBox &a, const BBox &b) { return a.score > b.score; });

    int box_num = input.size();

    std::vector<int> merged(box_num, 0);

    for (int i = 0; i < box_num; i++) {
        if (merged[i])
            continue;
        std::vector<BBox> buf;

        buf.push_back(input[i]);
        merged[i] = 1;

        float h0 = input[i].y2 - input[i].y1 + 1;
        float w0 = input[i].x2 - input[i].x1 + 1;

        float area0 = h0 * w0;

        for (int j = i + 1; j < box_num; j++) {
            if (merged[j])
                continue;

            float inner_x0 = input[i].x1 > input[j].x1 ? input[i].x1 : input[j].x1;
            float inner_y0 = input[i].y1 > input[j].y1 ? input[i].y1 : input[j].y1;

            float inner_x1 = input[i].x2 < input[j].x2 ? input[i].x2 : input[j].x2;
            float inner_y1 = input[i].y2 < input[j].y2 ? input[i].y2 : input[j].y2;

            float inner_h = inner_y1 - inner_y0 + 1;
            float inner_w = inner_x1 - inner_x0 + 1;

            if (inner_h <= 0 || inner_w <= 0)
                continue;

            float inner_area = inner_h * inner_w;

            float h1 = input[j].y2 - input[j].y1 + 1;
            float w1 = input[j].x2 - input[j].x1 + 1;

            float area1 = h1 * w1;

            float score;

            score = inner_area / (area0 + area1 - inner_area);

            if (score > iou_threshold) {
                merged[j] = 1;
                buf.push_back(input[j]);
            }
        }
        switch (type) {
            case hard_nms: {
                output.push_back(buf[0]);

                break;
            }
            case blending_nms: {


                float total = 0;
                for (int i = 0; i < buf.size(); i++) {
                    total += exp(buf[i].score);
                }
                BBox rects;
                memset(&rects, 0, sizeof(rects));
                for (int i = 0; i < buf.size(); i++) {
                    float rate = exp(buf[i].score) / total;
                    rects.x1 += buf[i].x1 * rate;
                    rects.y1 += buf[i].y1 * rate;
                    rects.x2 += buf[i].x2 * rate;
                    rects.y2 += buf[i].y2 * rate;
                    rects.score += buf[i].score * rate;
                }
        rects.label = buf[0].label;
                output.push_back(rects);
                break;
            }
            default: {
                printf("wrong type of nms.");
                exit(-1);
            }
        }
    }



    }
}

class CarInfo{

public:
    static int count;
public:
    int x;
    int y;
    int num;
    chrono::steady_clock::time_point timeStamp;


    CarInfo(){
        x = 0;
        y = 0;
        num = 0;

    }
};
int  CarInfo:: count = 0;

//int detect_region_start_Y = 100;
//int detect_region_end_Y = 400;
//int car_leave_Y = 450;

int detect_region_start_Y = 600;
int detect_region_end_Y = 800;
int car_leave_Y = 850;

int main(){
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    CPU_SET(1, &mask);
    CPU_SET(2, &mask);
    CPU_SET(3, &mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);


    std::vector<CarInfo> carInfoVect;
    string base_string1 = "MVI_20011__img";
    string base_string2 = "new__img";

    // Creating Model
    const char *mnn_path = "quan.mnn";
    std::shared_ptr<MNN::Interpreter> interpreter(MNN::Interpreter::createFromFile(mnn_path));

    int num_thread = 4 ;
    MNN::ScheduleConfig config;
    config.numThread = num_thread;

    MNN::BackendConfig backendConfig;

    //backendConfig.precision = (MNN::BackendConfig::PrecisionMode) 0;
    backendConfig.precision = BackendConfig::Precision_Low;

    config.backendConfig = &backendConfig;

    auto session = interpreter->createSession(config);




    //Model Created
    int _processed_pic_num = 0;
    for(int picIndex = 1; picIndex<= 5199; picIndex++){
        _processed_pic_num++;
        char _str[6];
        sprintf(_str, "%05d", picIndex);
        //string _in_name ="./img_1/" + base_string1 + string(_str) + ".jpg";
        //string _out_name = "./img_2/" + base_string2 + string(_str) + ".jpg";
        string _in_name ="./img_3/" + string(_str) + ".jpg";
        string _out_name = "./img_5/" + string(_str) + ".jpg";
        std::cout << _in_name << endl;
        cv::Mat row_img = cv::imread(_in_name);
        if(row_img.empty()){
            continue;
        }
        else{
            std::cout << "Success Read " << picIndex << endl;
        }

        int iw = row_img.cols;
        int ih = row_img.rows;

        cv::Mat image;
        cv::resize(row_img, image,cv::Size(INPUT_SIZE,INPUT_SIZE));
        auto input_tensor = interpreter->getSessionInput(session, nullptr);
        interpreter->resizeTensor(input_tensor, {1,INPUT_SIZE,INPUT_SIZE,3}); //输入tensor格式NCHW
        interpreter->resizeSession(session);
        std::shared_ptr<MNN::CV::ImageProcess> pretreat(
           MNN::CV::ImageProcess::create(MNN::CV::BGR, MNN::CV::RGB, mean_vals, 3,norm_vals, 3));
        pretreat->convert(image.data, 256, 256,0, input_tensor);
        auto start =chrono::steady_clock::now();
        interpreter->runSession(session);
        std::string scores = "conv2d_25/BiasAdd";
        std::string scores2 = "conv2d_26/BiasAdd";

        MNN::Tensor *tensor_scores = interpreter->getSessionOutput(session, scores.c_str());
        MNN::Tensor *tensor_scores2 = interpreter->getSessionOutput(session, scores2.c_str());

        auto tensor_scores_host = new Tensor(tensor_scores, MNN::Tensor::TENSORFLOW);//NHWC
        tensor_scores->copyToHostTensor(tensor_scores_host);

        auto tensor_scores_host2 = new Tensor(tensor_scores2, MNN::Tensor::TENSORFLOW);
        tensor_scores2->copyToHostTensor(tensor_scores_host2);

        std::vector<MNN::Tensor *> output_tensor;
        output_tensor.push_back(tensor_scores_host);
        output_tensor.push_back(tensor_scores_host2);

        std::vector<BBox> row_boxes,dest_boxes;

        postprocess(output_tensor,row_boxes,iw,ih);
        nms(row_boxes,dest_boxes,0.35,hard_nms);

        auto end =chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;
        cout<<"postprocess time: " <<elapsed.count()<<endl;


        //carInfoVect
        for(auto box : dest_boxes){
            CarInfo _aCar;
            int _findMatched = 0;

            int _avX = 0, _avY = 0;
            _avX = (box.x1 + box.x2) / 2;
            _avY = (box.y1 + box.y2) / 2;
            _aCar.x = _avX;
            _aCar.y = _avY;


//            if(9*_avX - 8 * _avY - 5040 < 0 && _avY < detect_region_end_Y && _avY > detect_region_start_Y){

            int _carIndex = 0;
            //Maching this car to car in vector
            for(auto _car : carInfoVect)
            {
                int _dx = _car.x - _aCar.x;
                int _dy = _car.y - _aCar.y;
                if(_dx*_dx + _dy*_dy < 40000)
                {
                    _findMatched =1;
                    break;
                }
                _carIndex++;
            }
            if(_findMatched == 0)//Have Not Found old car matchinig this car
            {
                if(41*_avX - 192*_avY + 59520 < 0 && _avY < detect_region_end_Y && _avY > detect_region_start_Y){
                    CarInfo::count++;
                    _aCar.num = CarInfo::count;
                    _aCar.timeStamp = chrono::steady_clock::now();
                    carInfoVect.push_back(_aCar);
                    std::cout << "ADD Car Num: " << _aCar.num << ", y: " << _aCar.y << endl;
                }
            }
            else
            {
                //Update Position
                carInfoVect[_carIndex].x = _aCar.x;
                carInfoVect[_carIndex].y = _aCar.y;
                carInfoVect[_carIndex].timeStamp = chrono::steady_clock::now();
                //If The Found Car Almost Go Out Of Picture
                if(_avY > car_leave_Y){
                    std::cout << "Erase Car Num: " << carInfoVect[_carIndex].num << endl;
                    carInfoVect.erase(carInfoVect.begin()+_carIndex); //Delete This Car
                }
            }

            cv::Point pt1(box.x1, box.y1);
            cv::Point pt2(box.x2, box.y2);
            CvFont _font;
            cvInitFont(&_font, CV_FONT_HERSHEY_PLAIN, 1.5f, 1.5f, 0, 2, CV_AA);//设置显示的字体

            cv::rectangle(row_img, pt1, pt2, cv::Scalar(0, 255, 0), 2);

            if(_findMatched == 0){
                cv::putText(row_img, to_string(_aCar.num), cv::Point(box.x1, box.y1-10), 6, 1, cv::Scalar(0, 0, 255), 2);
            }

        }
        int _vec_size = carInfoVect.size();
        chrono::duration<double> _car_stay_time;
        if(_processed_pic_num % 10 == 0){
            for(int _i = 0; _i < _vec_size; _i++){
                _car_stay_time = end - carInfoVect[_i].timeStamp;
                if(_car_stay_time.count() > 12){ //5 Seconds Pass and the car's timestamp has not been updated
                    std::cout << "Erase Car Num (overtime): " << carInfoVect[_i].num << endl;
                    carInfoVect.erase(carInfoVect.begin()+_i); //Delete This Car
                    _vec_size = carInfoVect.size();
                }
            }
        }
        /*cv::line(row_img, cv::Point(560, 0), cv::Point(959, 450), cv::Scalar(0, 0, 255));
        cv::line(row_img, cv::Point(0, detect_region_start_Y), cv::Point(959, detect_region_start_Y), cv::Scalar(0, 0, 255));
        cv::line(row_img, cv::Point(0, detect_region_end_Y), cv::Point(959, detect_region_end_Y), cv::Scalar(0, 0, 255));
        cv::line(row_img, cv::Point(0, car_leave_Y), cv::Point(959, car_leave_Y), cv::Scalar(0, 0, 255));*/
        /*cv::line(row_img, cv::Point(0, 310), cv::Point(1919, 720), cv::Scalar(0, 0, 255));
        cv::line(row_img, cv::Point(0, detect_region_start_Y), cv::Point(1919, detect_region_start_Y), cv::Scalar(0, 0, 255));
        cv::line(row_img, cv::Point(0, detect_region_end_Y), cv::Point(1919, detect_region_end_Y), cv::Scalar(0, 0, 255));
        cv::line(row_img, cv::Point(0, car_leave_Y), cv::Point(1919, car_leave_Y), cv::Scalar(0, 0, 255));*/
        cv::putText(row_img, "Car number: "+to_string(CarInfo::count), cv::Point(100, 100), 1, 3, cv::Scalar(0, 0, 255), 4);
        cv::putText(row_img, to_string((int)(1/elapsed.count()))+"FPS", cv::Point(100, 150), 1, 3, cv::Scalar(0, 0, 255), 4);

        cv::imwrite(_out_name, row_img);
        std::cout << "Vector Size: " << carInfoVect.size() << endl;
        std::cout << "Car Count: " << CarInfo::count << endl;
        //cv::waitKey(0);
    }
    return 1;
    cv::Mat row_img = cv::imread("MVI_20011__img00001.jpg");




}
