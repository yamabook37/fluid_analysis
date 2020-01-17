#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/*
最終更新：1/17 2:00
編集者：山本
基本的な変数，定数をコピー

*/

#define NX (51)
#define NY (51)
#define KU (1.0)
#define mu (0.20)//計算の安定性を決めるファクター mu > 2.5 だと計算が爆発する


int main(){
  int nx = NX, ny = NY, icnt = 0;
  double f[NY][NX], fn[NY][NX], dt,
    Lx = 1.0,  Ly = 1.0, kappa = KU, t = 0.0,
    dx = Lx/(double)(nx-1), dy = Ly/(double)(ny-1);
  FILE *fp;
  
  return 0;
}