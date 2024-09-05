# 制作数据集
制作方法见[tools目录下的prepare_dataset说明文档](../../tools/prepare_dataset/README.md)

# 训练准备
该部分训练使用了OpenMMLab提供的[mmyolo](https://github.com/open-mmlab/mmyolo)

这里提供本机的版本方便对应，不同版本的mmyolo需要安装指定版本的mmcv,mmdet等，具体请参考mmyolo的文档。

> cuda                                 11.7  
> torch                                1.13.1
> mmengine                             0.10.4      
> mmcv                                 2.0.0rc4 
> mmdet                                3.3.0       
> mmyolo                               0.6.0      

# 训练
训练配置文件见yolov5/train_config.py，详细配置参考[mmyolo文档](https://mmyolo.readthedocs.io/zh-cn/latest/)
具体的训练命令如下：
```bash
python src/train_model/train.py src/train_model/yolov5/train_config.py --resume
```

训练得到的.pth模型文件在`src/train_model/work_dirs`下

# 推理
如果希望输出图片，需要加入`--out-dir dataset/single_object/pred_images`，同时删除`--show`。

```bash
python src/train_model/image_demo.py  dataset/single_object/images/\
                           src/train_model/yolov5/train_config.py \
                          src/train_model/work_dirs/best_coco_bbox_mAP_epoch_136.pth \
                          --show
```
