#最終更新： 2020/1/31 3:30
#編集者：武者野

'''
[previous 主な更新内容]
・保存ファイルの指定を簡単化する設定を追加 <- いいね

[主な改善点]

[改善点]
・タイトルと中身以外の ax は使い回したい
・y軸ラベルのYはみだし問題

気になったセンスのある人は配置とか配色とかフォントとか透明度とかいじってみてほしい

[次の予定]
・κの値を変えたものを同時上映（ax 増やせばすぐできる）
・計算条件から FILE_PATH を決定するのを自動化（Cで出力するようにすればすぐできる）

[お気持ち]
κ、μと時間の表示がちょっとうるさいかもしれない。
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

###################### ANIME ###################
#ファイル名(含む：条件）を一括で指定
FILE_PATH = 'animes/ad_dif_point_k001_test'
###################### ANIME ###################

################### PARAMETER ##################
#図のタイトル (グローバル変数で良いよね？)
TITLE = '2D advection diffusion'
################### PARAMETER ##################

#matplotlibの設定確認
#print(matplotlib.matplotlib_fname())

picnum_file = open('data/picture_number.txt')
FRAME_NUMBER = int(picnum_file.readline())
# update 関数を呼び出す回数は（写真の枚数 - １）∵init で一枚使う
# と思ったんだけどなぜか2枚引かないとエラーになる。何でか分かったら教えて
FRAME_NUMBER -= 2

f = open('data/advection_diffusion_time.txt')
line = f.readline()
NX, NY = map(int, line.split())
line = f.readline()

kappa, mu = map(float, line.split())

#print(NX,NY,k)

# 図のサイズ（インチ）
fig = plt.figure(figsize=(5, 5))
# y軸ラベルはみ出し問題対処
fig.subplots_adjust(left=0.80)
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

  #タイトルも関数でまとめて
  ax.set_title(TITLE)

  #text を一つにした、バックグラウンドカラーを黒くした
  #transform = ax.transAxes とすることで、挿入位置を pixel でなく座標で指定
  #こうすることで画像の大きさ（pixel数）が変わっても同じ場所に配置できる。以下参考
  #http://ars096.hatenablog.com/entry/2013/08/16/011727
  ax.text(0.02, 0.035, r"$\mu$="+str(mu)+"\n"
                     +r"$\kappa$="+str(kappa),
          size=20, color="white", backgroundcolor='black', transform = ax.transAxes)

#文字の重なりを防止
plt.tight_layout()

# はじめに呼び出す関数
def init(fig):
  time = float(f.readline())

  #時間以外はここでまとめて定義
  #ax_set の呼び出しがここでの一回で済ませられたら嬉しい
  ax_set(ax)
  ax.text(0.02, 0.92, 'time = ' + str(time), size=20, color="white",
          backgroundcolor='black', transform=ax.transAxes)
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
  ax.text(0.02, 0.92, 'time = ' + str(time), size=20, color="white",
          backgroundcolor='black', transform=ax.transAxes)
  
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
  ani.save(FILE_PATH + ".mp4", writer="ffmpeg")
# gif ファイルとして保存 yamamoto 用
if(YAMAMOTO_GIF == 1):
  ani.save(FILE_PATH + ".gif", writer="imagemagick")
# gif ファイルとして保存 mushano 用
if(MUSHANO_GIF == 1):
  ani.save(FILE_PATH + ".gif", writer="pillow")
if(HTML_SHOW == 1):
  HTML(ani.to_jshtml()) # HTML上で表示

f.close
picnum_file.close
