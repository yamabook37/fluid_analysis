'''
↓参考
https://matplotlib.org/examples/pylab_examples/quiver_simple_demo.html


==================================================
A simple example of a quiver plot with a quiverkey
==================================================
'''
import matplotlib.pyplot as plt
import numpy as np

X = np.arange(-10, 10, 1)
Y = np.arange(-10, 10, 1)

"""
下の、np.meshgrid(X, Y)

各成分ごとのｘ値とy値を埋めたものを生成してくれる,例えば
x = [1,2,3,4,5,6,7,8,9]
y = [10,20,30,40,50]

X,Y = np.meshgrid(x,y)
print(X)
=> [[1 2 3 4 5 6 7 8 9]
     [1 2 3 4 5 6 7 8 9]
     [1 2 3 4 5 6 7 8 9]
     [1 2 3 4 5 6 7 8 9]
     [1 2 3 4 5 6 7 8 9]]

print(Y)
 => [[10 10 10 10 10 10 10 10 10]
     [20 20 20 20 20 20 20 20 20]
     [30 30 30 30 30 30 30 30 30]
     [40 40 40 40 40 40 40 40 40]
     [50 50 50 50 50 50 50 50 50]]

という感じ。

     """


U, V = np.meshgrid(X, Y)

fig, ax = plt.subplots()
q = ax.quiver(X, Y, U, V)

#ベクトルで描画する
#plt.quiverkey(Q, x位置,y位置,長さ,単位（文字）)
#labelposはreference allowに対するラベル（文字）の位置。N,S,E,Wで指定。
ax.quiverkey(q, X=0.3, Y=1.1, U=10,
             label='Quiver key, length = 10', labelpos='E')

plt.show()
