# 制作物料数据集
## 使用OpenCV获取原始图像
详细代码见[prepare_dataset.cpp](prepare_dataset.cpp)

使用以下命令编译源代码
```bash
mkdir build
cd build
cmake .. && make
```

运行程序（确认已安装摄像头且路径在build/下）
```bash
./prepare_dataset ../../../dataset/objects/images
```

获取的图片分辨率为640x480，保存在`GXCL2024/dataset/objects/images`下
## labelme 标注
使用labelme软件对图片进行标注。
```bash
conda create -n labelme python=3.8
conda activate labelme
pip install labelme==5.1.1
```
启动labelme软件
```bash
labelme ${图片文件夹路径（即上一步的图片文件夹）} \
        --output ${label文件所处的文件夹路径（即上一步的 --out-dir）} \
        --autosave \
        --nodata
```
**e.g.**
```bash
labelme ./dataset/objects/images --output ./dataset/objects/labels --autosave --nodata
```
**标注的时候务必使用 `rectangle`，快捷键 **Ctrl + R****
此时

## 标签预处理
在dataset/objects/下创建`class_with_id.txt`文件，内容为
```txt
1 blue
2 green
3 red
```

使用python脚本对label文件进行预处理
```bash
python ./labelme2coco.py --img-dir ${图片文件夹路径} \
                                                --labels-dir ${label 文件夹位置} \
                                                --out ${输出 COCO label json 路径} \
                                                [--class-id-txt ${class_with_id.txt 路径}]
```

检查label是否正确
```bash
python ./rowse_coco_json.py --img-dir ${图片文件夹路径} \
                                                --ann-file ${COCO label json 路径}
```
## 数据集处理
### 数据集可视化
通过以下命令可视化数据集，可以得到数据集的统计信息，包括Object大小，数据集类别中的数量，
bbox 宽高比例范围，bbox 宽高比分布等
```bash
python ./dataset_analysis.py ${CONFIG} \
                                                [--val-dataset ${TYPE}] \
                                                [--class-name ${CLASS_NAME}] \
                                                [--area-rule ${AREA_RULE}] \
                                                [--func ${FUNC}] \
                                                [--out-dir ${OUT_DIR}]
```
### 优化Anchor
该步骤仅适用于 anchor-base 的模型，例如 YOLOv5；
该部分内容参考[优化锚框尺寸](https://mmyolo.readthedocs.io/zh-cn/latest/useful_tools/optimize_anchors.html)
```bash
# use K-means to generate anchor
python ./coco_split.py --json ${COCO label json 路径} \
                                --out-dir ${划分 label json 保存根路径} \
                                --ratios ${划分比例} \
                                [--shuffle] \
                                [--seed ${划分的随机种子}]
```
根据得到的anchor，替换配置文件中的 anchor。（注意[]的使用）
### 划分数据集
这里将数据集划分成训练集和验证集，验证集占训练集的1/4。
```bash
python tools/misc/coco_split.py --json ${COCO label json 路径} \
                                --out-dir ${划分 label json 保存根路径} \
                                --ratios 0.8 0.2 \
                                [--shuffle] \
                                [--seed 10]
```


## 模型训练
训练方法见[src中train_model下的README.md](../../src/train_model/README.md)


