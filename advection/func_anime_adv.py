#最終更新： 2020/3/22 4:00
#編集者：山本

'''
[主な更新内容]
・FuncAnimation を使ってアニメ制作
・写真のdataを受け取るファイルとアニメを出力するファイルを別フォルダにした
・写真を作る際の写真の数を受け取れるようにした
・κ、μの表示の仕方を変えた
・時間の表示をタイトルと分離した
'''

import numpy as np
import matplotlib 
import matplotlib.pyplot as plt
import matplotlib.animation as animation
#from IPython.display import HTML

###################### CONFIG ##################
# 動画保存方法の config. 使いたい保存方法を１にする.
YAMAMOTO_MP4 = 0
YAMAMOTO_GIF = 1
MUSHANO_GIF  = 0
HTML_SHOW    = 0
PLT          = 1


##################### ANIME ###################
# ファイル名(含む：条件）を一括で指定
# コード_表示する変数_初期条件_パラメータ
FILE_PATH = 'animes/adv_u_point_Re2000'


################### PARAMETER ##################
TITLE = '2D diffusion'

#matplotlibの設定確認
#print(matplotlib.matplotlib_fname())
picnum_file = open('data/picture_number.txt')
FRAME_NUMBER = int(picnum_file.readline())
# update 関数を呼び出す回数は（写真の枚数 - １）∵init で一枚使う
# と思ったんだけどなぜか2枚引かないとエラーになる。何でか分かったら教えて
FRAME_NUMBER -= 2

f = open('data/advection.txt')
NX, NY = map(int, f.readline().split())
kappa, mu = map(float, f.readline().split())


fig = plt.figure(figsize=(6, 6)) #図のサイズ（インチ）
fig.subplots_adjust(left=0.20)
ax = fig.add_subplot(111) # Axesを追加
ax.set_xlim(0, NX-1)
ax.set_ylim(0, NY-1)
ax.set_xlabel('X', fontsize=16)
ax.set_ylabel('y', fontsize=16)
ax.tick_params(labelsize=14)
ax.set_title(TITLE, fontsize=20)
ax.text(0.02, 0.035, r"$\mu$="+str(mu)+"\n"+r"$\kappa$="+str(kappa),
          size=20, color="white", backgroundcolor='black', transform = ax.transAxes)


time_text = ax.text(0.02, 0.91, '', size=20, color="white",
          backgroundcolor='black', transform=ax.transAxes)


################### 初期描画 ##################
time = float(f.readline())
time_text.set_text("time = " + str(time))

data=[]
for _ in range(NY):
  tmp = list(map(float, f.readline().split()))
  data.append(tmp)

im = ax.imshow(data, animated=True, cmap='jet')
cbar = fig.colorbar(im, shrink=0.7)
cbar.ax.tick_params(labelsize=16)


################### 描画 ##################
# 更新用関数, ここで表示させる
def update(cnt):
  if cnt == 0:
    return
  
  #print("cnt:",cnt)
  time = float(f.readline())
  
  # 二次元配列受け取り
  data=[]
  for _ in range(NY):
    tmp = list(map(float, f.readline().split()))
    data.append(tmp)
  
  # 変更箇所だけset
  time_text.set_text("time = " + str(time))
  im.set_data(data)

ani = animation.FuncAnimation(fig, update, interval=30, frames=FRAME_NUMBER)
  #↓↓↓↓ これらの注釈分かりやすい！！！！
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
if(PLT == 1):
  plt.show()

f.close
picnum_file.close