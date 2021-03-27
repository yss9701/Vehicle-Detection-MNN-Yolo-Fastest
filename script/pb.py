import tensorflow as tf
from tensorflow.python.framework import graph_util
#import tensorflowjs as tfjs

sess = tf.Session()
output_graph_def = tf.GraphDef()
#   feathers starry candy
PB_PATH = r"model_256.pb"
#TFJS_PATH = r'./tfjs/candy'

in_image = tf.placeholder(tf.float32, (1, 256, 256, 3), name='input_1')
with open(PB_PATH, "rb") as f:
    output_graph_def.ParseFromString(f.read())
    tf.import_graph_def(
        output_graph_def,
        input_map={
            'input_1:0': in_image
        },
        name='',  # 默认name为import,类似scope
        # return_elements=['generator/mul:0']
    )
sess.run(tf.global_variables_initializer())
# = sess.graph.get_tensor_by_name("generator/output:0")
output_node_names = "conv2d_25/BiasAdd,conv2d_26/BiasAdd"

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    constant_graph = graph_util.convert_variables_to_constants(sess, sess.graph_def,
                                                               output_node_names=output_node_names.split(","))
    with tf.gfile.FastGFile("tmp.pb", mode='wb') as f:
        f.write(constant_graph.SerializeToString())

#tfjs.converters.tf_saved_model_conversion_pb.convert_tf_frozen_model(
#    "./pb/tmp.pb",
#    'generator/output',
#    TFJS_PATH
#)