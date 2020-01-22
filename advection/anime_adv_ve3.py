#最終更新： 2020/1/22 18:00
#編集者：武者野

'''
[主な更新内容]
・FuncAnimation を使ってアニメ制作
・ヒートマップを作る 2D data は毎回 update 関数の中で受け取る
・アニメの保存方法で毎回喧嘩するので config 作った
・軸ラベルなどを一気に設定する ax_set 関数を作った
・update 関数の引数はとりあえず一つにした
・グラフタイトルをグローバル変数にした

[改善点]
・その１
ax の体裁はタイトル以外変わらないから、これを update 関数の中で毎回作りなおすのは無駄。
だからタイトル以外のところは使い回すようにしたいがよく分からん。
（まあそんな時間気にならないし別に良いかもしれない）
・その２
y軸ラベルの Y がどうしても図からはみ出してしまう
fig = plt.figure(figsize=(5, 5))
のインチ数をいじっても効果なし😥

[お気持ち]
出来栄えには結構満足している。
κの値変えたものを同時上映、とかもCの出力変えれば
すぐ対応できると思う
'''

import numpy as np
import matplotlib 
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from IPython.display import HTML

###################### CONFIG ##################
# 動画保存方法の config。 使いたい保存方法を１にする
YAMAMOTO_MP4 = 0
YAMAMOTO_GIF = 0
MUSHANO_GIF  = 1
HTML_SHOW    = 0
###################### CONFIG ##################

################### PARAMETER ##################
#何枚の写真まで受け取って動画にするか <- 本当は C で画像が何枚になるか計算し、それを受け取るべき
FRAME_NUMBER = 500
#図のタイトル (グローバル変数で良いよね？)
TITLE = '2D advection'
################### PARAMETER ##################

#matplotlibの設定確認
#print(matplotlib.matplotlib_fname())

f = open('advection.txt')
line = f.readline()
NX, NY = map(int, line.split())
line = f.readline()

kappa, mu = map(float, line.split())

#print(NX,NY,k)

#図のサイズ（インチ）
fig = plt.figure(figsize=(5, 5))
# Axesを追加
ax = fig.add_subplot(111)

#図の体裁を整える関数
def ax_set(ax):
  #表示範囲
  ax.set_xlim(0, NX-1)
  ax.set_ylim(0, NY-1)
  #体裁
  ax.set_xlabel('X')
  ax.set_ylabel('y')

  # Axesにテキストを追加
  # なんか知らんがTeX使うときは r が必要っぽい
  ax.text(0.5, 10.0, r"$\mu$="+str(mu), size=20, color="white")
  ax.text(0.5, 0.5, r"$\kappa$="+str(kappa), size=20, color="white")

#文字の重なりを防止
plt.tight_layout()

# はじめに呼び出す関数
def init(fig):
  time = float(f.readline())

  #タイトル以外の体裁は関数でまとめて定義
  #ax_set の呼び出しがここでの一回で済ませられたら嬉しい
  ax_set(ax)
  ax.set_title(TITLE + '  time=' + str(time))

  data=[]
  for j in range(NY):
    tmp = list(map(float, f.readline().split()))
    data.append(tmp)

  im = ax.imshow(data, animated=True, cmap='jet')
  fig.colorbar(im, shrink=0.8)  #縮尺を無理くり図のサイズに合わせた

# 更新用関数, ここで表示させる
def update(cnt):
  if cnt != 0:
    plt.cla()  # 現在描写されているグラフを消去 -> 毎回同じaxも全部clearされてしまっている
  
  time = float(f.readline())

  #ここにplt関係を書いてしまう
  
  ax_set(ax)  # ここは毎回同じだから、できればここだけ使い回すようにしたい
  ax.set_title(TITLE + '  time=' + str(time))
  
  #二次元配列受け取り
  data=[]
  for j in range(NY):
    tmp = list(map(float, f.readline().split()))
    data.append(tmp)

  im = ax.imshow(data, animated=True, cmap='jet')

  return [im]

ani = animation.FuncAnimation(fig, update,
                              init_func=init(fig), interval=100, frames=FRAME_NUMBER)

  #↓↓↓↓ これらの注釈分かりやすい！！！！
  #blit=Trueすると動かなくなる
  #第二引数：コールバック用関数（Artist:図を複数回呼び出すに対し，Func:図を作成する関数を複数回呼び出す）
  #オプション fargs 複数の引数 <-- なるほど！とりあえず今回は引数は一つにした
  #オプション frames 何枚の画像か，つまり関数を呼び出す回数 <-- なるほど！

# mp4 ファイルとして保存 yamamoto 用
if(YAMAMOTO_MP4 == 1):
  ani.save("advection_anime_3.mp4", writer="ffmpeg")
# gif ファイルとして保存 yamamoto 用
if(YAMAMOTO_GIF == 1):
  ani.save("advection_anime_3.gif", writer="imagemagick")
# gif ファイルとして保存 mushano 用
if(MUSHANO_GIF == 1):
  ani.save("advection_anime_3.gif", writer="pillow")
if(HTML_SHOW == 1):
  HTML(ani.to_jshtml()) # HTML上で表示

f.close
