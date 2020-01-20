#最終更新2020/1/16 6:00
'''
更新内容
時間を表示させる方法についてかなり粘ったけどなかなかうまくいかんなあ
ArtistAnimation じゃなくて FuncAnimation を使った方が良いんだろうけど、
まず FuncAnimation の使い方がいまいち分からん。
https://qiita.com/MENDY/items/e1b432df1e0bfe8b680c
http://intothelambda.com/archives/8
この人達が出来てるから出来るんだろうけど、
imshow と FuncAnimation を使ってる例が見付からん

文字色を黒に変更
'''

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
#from IPython.display import HTML

f=open('advection.txt')
#f=open('sample2.txt')
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
ax.text(0.5, 10.0, r"$\mu$="+str(mu), size = 20, color = "black")
ax.text(0.5, 0.5, r"$\kappa$="+str(kappa), size = 20, color = "black")

#文字の重なりを防止
plt.tight_layout()

ims=[]
cnt_image=0
line=f.readline()
while line: #lineがある間実行
    time=float(line)

    #二次元配列
    data=[]

    for j in range(NY):
        tmp = list(map(float, f.readline().split()))
        data.append(tmp)
    
    # このテキスト処理のせいで実行速度かなり遅くなる，2minはかかる
    # いらない場合はコメントアウト推奨
    # 何回も表示するから重なって見えてしまう
    #ax.text(0.5, 4, "time="+str(time), size = 14, color = "white", family='monospace', withdash=True)
    
    '''
    if cnt_image == 5:
        break
    '''

    #配列をちゃんと受け取れているかの確認用
    #print(*data, sep='\n')

    #plt.imshow()の返り値はlist型ではない（plt.plot()の返り値はlist型）
    im = ax.imshow(data, animated=True, cmap='jet')
    #fig.colorbar(im)
    
    if cnt_image == 0:  #最初のループだけカラーバーをつける
        fig.colorbar(im, shrink=0.8) #縮尺を無理くり図のサイズに合わせた
        
    #imsにappendするのはlist型で無ければならない
    ims.append([im])
    cnt_image += 1

    #plt.show()

    #これが無かったからcntが変わらなかった
    line = f.readline()

#型の確認
print(type(data))
print(type(ims))
print("number of images:", cnt_image)


#第二引数はリスト
ani = animation.ArtistAnimation(fig, ims, interval=10, repeat_delay=100, blit=True,)
ani.save("advection_anime.mp4", writer="ffmpeg") # mp4 ファイルとして保存 yamamoto 用
#ani.save("Animation1.gif", writer="imagemagick") # gif ファイルとして保存
#ani.save("advection_anime.gif", writer="pillow") # gif ファイルとして保存 nushano 用

'''
ani.save("Animation1.gif", writer='pillow') # gif ファイルとして保存
HTML(ani.to_jshtml()) # HTML上で表示
'''

f.close