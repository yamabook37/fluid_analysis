#最終更新2020/1/16 2:48

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
#from IPython.display import HTML

f=open('diffusion_ver2.txt')
#f=open('sample2.txt')
line=f.readline()
NX ,NY =map(int, line.split())
line = f.readline()
kappa = float(line)

#print(NX,NY,k)


#図のサイズ（インチ）
fig = plt.figure(figsize=(5, 5))
#表示範囲
plt.xlim(0, NX-1)
plt.ylim(0, NY-1)
#体裁
plt.xlabel('X')
plt.ylabel('y')
plt.title('2D_diffusion')

# Axesを追加
ax = fig.add_subplot(111)
# Axesにテキストを追加
ax.text(0.5, 0.5, "kappa="+str(kappa), size = 14, color = "white")
#文字の重なりを防止
plt.tight_layout()

ims=[]
cnt_image=0
line=f.readline()
while line: #lineがある間実行
    time=float(line)
    data=[]

    for j in range(NY):
        tmp = list(map(float, f.readline().split()))
        data.append(tmp)
    
    #このテキスト処理のせいで実行速度かなり遅くなる，2minはかかる
    # いらない場合はコメントアウト推奨
    # 何回も表示するから重なって見えてしまう
    #ax.text(0.5, 4, "time="+str(time), size = 14, color = "white", family='monospace', withdash=True)
    '''
    if cnt_image ==5:
        break
    '''

    #print(*data, sep='\n')
    im=plt.imshow(data, animated=True, cmap='jet')
    #fig.colorbar(im)
    if cnt_image == 0:  #最初のループだけカラーバーをつける
        fig.colorbar(im, shrink=0.8) #縮尺を無理くり図のサイズに合わせた
    
    ims.append([im])
    cnt_image += 1

    #plt.show()

    #これが無かったからcntが変わらなかった
    line = f.readline()

#型の確認
print(type(data))
print(type(ims))
print("number of images:", cnt_image)



ani = animation.ArtistAnimation(fig, ims, interval=10, repeat_delay=100, blit=True,) #第二引数はリスト
ani.save("diffusion_anime.mp4", writer="ffmpeg") # mp4 ファイルとして保存
#ani.save("Animation1.gif", writer="imagemagick") # gif ファイルとして保存
#ani.save("diffusion_anime.gif", writer="pillow") # gif ファイルとして保存

'''
ani.save("Animation1.gif", writer='pillow') # gif ファイルとして保存
HTML(ani.to_jshtml()) # HTML上で表示
'''

f.close