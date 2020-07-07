Extrinsic Calibration for multiple sensor system using High-density panorama 3d data

## ビルド方法

git clone https://github.com/cln515/GUI-Calibration --recursive

その後　CMake（依存: OpenCV, Ceres Solver） 

## キャリブレーション方法

### 三次元データの変換

三次元データ：ptx形式

"Open Ptx"でptxファイルを指定後、"Generate Panorama"で画像に変換。"Generate Panorama"で二回ダイアログが出てくるので色画像とデプス画像の保存先(拡張子まで必要)を指定。色画像が表示される。

### カメラデータの読み込み

カメラデータ　json形式
```rb
{
  "0": {
    "fx": 357.5809711,
    "fy": 356.45315436,
    "cx": 627.68586992,
    "cy": 499.44184605,
    "k1": 0.00616474,
    "k2": -0.00601809,
    "k3": 0.00421195,
    "k4": -0.0010912
  },


  "1": {
    "fx": 357.23731439,
    "fy": 356.11931836,
    "cx": 617.38230989,
    "cy": 475.52586598,

    "k1": 0.0030682,
    "k2": 0.00048328,
    "k3": -0.00082956,
    "k4": 0.00028397
  }
}
```

"Open Camera Data” でjsonファイルを指定。

### 画像ファイルの読み込み
"Open Image" で出るダイアログで画像ファイルを指定

### 特徴点抽出
"FP Ext" で特徴点の抽出を行う

### 対応点指定
スペースキーを押しながらクリックで近傍の特徴点を指定。両画像で特徴点を指定したらsキーで保存する。特徴点対応を消したいときはdキーで最後の特徴点を消去する。

### 対応点保存読み込み
対応点を取ったカメラのid(json fileと対応)とpositionのid(0から順番につける)を指定して"Set Match"で保存する。"Load Match"で指定されているcamera idとposition idの場所、特徴点が読み込まれる。

### キャリブレーション
保存した対応点からキャリブレーションを行う。結果はコマンドラインに出力される

### 保存と読み込み
"Save Json"でファイルのパスや対応点を保存、"Open Json"で保存したデータを





