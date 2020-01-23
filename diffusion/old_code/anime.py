import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# 配列の情報を見るために用意した関数
def array_info(x):
    print("配列のshape", x.shape)
    print("配列の要素のデータ型", x.dtype)
    if len(x) >=10:
        print("配列の中身（上から10列）\n",x[:10],"\n")
    else:
        print("配列の中身\n",x,"\n")


#ファイルからの読み込み
data = np.loadtxt("./sample3.txt", 
            dtype='float', 
            skiprows=1, 
            unpack=True)
# np.loadtxtは二次元配列までしか扱えない，51*12801の配列になってる
# [[x1,x2,x3...,xn], [y1,...]]とするといい

#要素数が変わらないように変形
redata = data.reshape([2, 51, 51])
    #redata[0],redata[1]
a0,a1 = np.array_split(redata,2,0)

#print(type(a0))
print(a0)
print(type(data))
#array_info(data)
#array_info(redata)


#図のサイズ（インチ）
plt.figure(figsize=(5, 5))
#表示範囲
plt.xlim(0, 50)
plt.ylim(0, 50)
#体裁
plt.xlabel('X')
plt.ylabel('y')
plt.title('2D_diffusion')
#plt.gca().set_aspect('equal', adjustable='box')

#カラーバーの追加
#plt.colorbar()

#値の入力
#plt.plot(x,y, label='kappa')
#kappaに変数を入れる

#2次元カラーマップにするならZも必要
#plt.pcolormesh(data[0],data[1],data[5,:,:],cmap='jet')
plt.imshow(data, cmap='jet')
#plt.imshow(a0[0], cmap='jet')

#凡例の表示
#plt.legend()
#表示設定の反映，表示
plt.show()


#アニメにする時
# 書く時間のグラフを保存しておいてArtistanimationで表示する