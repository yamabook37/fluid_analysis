#最終更新： 2020/1/22 2:00
#編集者：山本

'''
更新内容
FuncAnimationに対応
imshow と FuncAnimation を使ってる例が見付からん
  imshowは図オブジェクトを作成し繋げるが，今回は関数によって繋げる
インデントをスペース2個に変更

改善したい点
時間を表示させる方法
'''

import numpy as np
import matplotlib 
import matplotlib.pyplot as plt
import matplotlib.animation as animation
#from IPython.display import HTML

#あとで消す
#x = np.arange(0, 10, 0.1)


# はじめに呼び出す関数
def init(fig, im):
  fig.colorbar(im, shrink=0.8)  #縮尺を無理くり図のサイズに合わせた
  # check OK


# 更新用関数, ここで表示させる
def update(cnt, fig_title, tt, kappa):
  if cnt != 0:
    plt.cla()  # 現在描写されているグラフを消去

  #ここにplt関係を書いてしまう
  plt.title(fig_title + 'cnt=' + str(cnt) + '  time=' + str(tt[cnt])) #関数を呼ぶ回数分時間を表示できる
    #dataのcntは50まで，timeのcntは253まで
  im = ax.imshow([list(data[cnt])], animated=True, cmap='jet')
  #ims_out = [ims[cnt]]

  return [im]

  '''
  y = kappa * np.sin(x - cnt)
  plt.plot(x, y, "r")
  plt.title(fig_title + 'cnt=' + str(cnt) + '  kappa=' + str(kappa))
  '''
  


#matplotlibの設定確認
#print(matplotlib.matplotlib_fname())


f=open('advection.txt')
line=f.readline()
NX ,NY =map(int, line.split())
line = f.readline()

# 注：muの受け取り追加 1/16 14:00
kappa, mu = map(float, line.split())

#print(NX,NY,k)

#図のサイズ（インチ）
fig = plt.figure(figsize=(5, 5))
# Axesを追加
ax = fig.add_subplot(111)

#表示範囲
ax.set_xlim(0, NX-1)
ax.set_ylim(0, NY-1)
#体裁
ax.set_xlabel('X')
ax.set_ylabel('y')
ax.set_title('2D_advection')

# Axesにテキストを追加
# なんか知らんがTeX使うときは r が必要っぽい
ax.text(0.5, 10.0, r"$\mu$="+str(mu), size = 20, color = "white")
ax.text(0.5, 0.5, r"$\kappa$="+str(kappa), size = 20, color = "white")

#文字の重なりを防止
plt.tight_layout()

tt=[]
ims=[]
cnt_image=0
line=f.readline()
while line: #lineがある間実行
  time=float(line)
  tt.append(time)

  #二次元配列
  data=[]

  for j in range(NY):
    tmp = list(map(float, f.readline().split()))
    data.append(tmp)
    
  # このテキスト処理のせいで実行速度かなり遅くなる，2minはかかる
  # いらない場合はコメントアウト推奨
  # 何回も表示するから重なって見えてしまう
  #ax.text(0.5, 4, "time="+str(time), size = 14, color = "white", family='monospace', withdash=True)
    

  #配列をちゃんと受け取れているかの確認用
  #print(*data, sep='\n')

  #画像の出力
  #plt.imshow()の返り値はlist型ではない（plt.plot()の返り値はlist型）
  im = ax.imshow(data, animated=True, cmap='jet')
        
  # AxesImageオブジェクト型を、listにしてからlistに追加
  ims.append([im]) #imsは全てのフレームを含んだリスト
  cnt_image += 1

  #plt.show()

  #これが無かったからcntが変わらなかった
  line = f.readline()
#while end


#型の確認
#print(type(data))
#print(type(ims))
print(tt[10])
print("number of images:", cnt_image)


#第二引数はリストの必要あり？
#ani = animation.ArtistAnimation(fig, ims, interval=10, repeat_delay=100, blit=True)
    # blitをTrueにするとfig_imageを返す時にリストだとできなくなる（複数描画が困難な可能性）
    # imgにappendしていく形をとる場合は，blitをFalseにするように

# Func
ani = animation.FuncAnimation(fig, update, fargs = ('2D_advection  ', tt, kappa),
  init_func=init(fig,im), interval = 100, frames = 50)

  #blit=Trueすると動かなくなる
  #第二引数：コールバック用関数（Artist:図を複数回呼び出すに対し，Func:図を作成する関数を複数回呼び出す）
  #オプション fargs 複数の引数
  #オプション frames 何枚の画像か，つまり関数を呼び出す回数

#ani.save("advection_anime_2.mp4", writer="ffmpeg") # mp4 ファイルとして保存 yamamoto 用
ani.save("advection_anime_2.gif", writer="imagemagick") # gif ファイルとして保存 yamamoto 用
#ani.save("advection_anime_2.gif", writer="pillow") # gif ファイルとして保存 mushano 用

#HTML(ani.to_jshtml()) # HTML上で表示

f.close