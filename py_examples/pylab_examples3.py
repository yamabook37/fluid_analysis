'''
↓参考
https://matplotlib.org/examples/pylab_examples/quiver_demo.html


========================================================
Demonstration of advanced quiver and quiverkey functions
========================================================

Known problem: the plot autoscaling does not take into account
the arrows, so those on the boundaries are often out of the picture.
This is *not* an easy problem to solve in a perfectly general way.
The workaround is to manually expand the axes.
'''
import matplotlib.pyplot as plt
import numpy as np
from numpy import ma

X, Y = np.meshgrid(np.arange(0, 2 * np.pi, .2), np.arange(0, 2 * np.pi, .2))
U = np.cos(X)
V = np.sin(Y)

"""
quiverの中身

units:単位をどのように定めるかを指定する.
width/height   描画領域（軸）の幅/高さを1とする。図の拡大にあわせて比を保つ。
dots/inches    図のdpiに基づいたピクセル/インチ
x/y/xy         x軸/y軸/sqrt(x**2+y**2)の1単位を1とする
矢印の色も指定できる

pivot:グリッドに矢印のどこを合わせるか。 ‘tail’, ‘middle’ ,‘tip’が選べる.
width:矢印の柄の部分の幅

"""
plt.figure()
plt.title('Arrows scale with plot width, not view')
Q = plt.quiver(X, Y, U, V, units='width')
qk = plt.quiverkey(Q, 0.9, 0.9, 2, r'$2 \frac{m}{s}$', labelpos='E',
                   coordinates='figure')

plt.figure()
plt.title("pivot='mid'; every third arrow; units='inches'")
Q = plt.quiver(X[::3, ::3], Y[::3, ::3], U[::3, ::3], V[::3, ::3],
               pivot='mid', units='inches')
qk = plt.quiverkey(Q, 0.9, 0.9, 1, r'$1 \frac{m}{s}$', labelpos='E',
                   coordinates='figure')

#点を打つ、ｓはサイズ、いろいろ変えられる
plt.scatter(X[::3, ::3], Y[::3, ::3], color='r', s=5)

plt.figure()
plt.title("pivot='tip'; scales with x view")
M = np.hypot(U, V)
Q = plt.quiver(X, Y, U, V, M, units='x', pivot='tip', width=0.022,
               scale=1 / 0.15)
qk = plt.quiverkey(Q, 0.9, 0.9, 1, r'$1 \frac{m}{s}$', labelpos='E',
                   coordinates='figure')
plt.scatter(X, Y, color='k', s=5)
