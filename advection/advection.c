/*
最終更新：1/19 22:00
編集者：山本
セグメンテーションエラーの原因を究明中
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NX (5+2)
#define NY (5+2)
#define KU (1.0)
#define mu (0.20)//計算の安定性を決めるファクター mu > 2.5 だと計算が爆発する

double MIN2(double x, double y);

//初期状態を決定
void initial(int nx, int ny, double dx, double dy, double f[][nx]);
//その時刻における f[jy][jx] の値をファイルとターミナルにアウトプット
void output(int nx, int ny, double f[][nx], double t, FILE *fp);

//f[jy][jx] をもとにワンタイムステップ後の状態 fn[jy][jx] を計算。
//fn は fn[0][jx] など（つまり境界）は更新されないことに注意
void advection(int nx, int ny, double f[][nx], double fn[][nx], double u, double v, double dt, double dx, double dy);

//fn に境界条件を課す
void boundary(int nx, int ny, double fn[][nx]);
//更新。計算して求めた fn は新たな fn を求めるための f へと♪～
void update(int nx, int ny, double f[][nx], double fn[][nx]);


//ほぼ変更せず，流れに沿って作成
int main(){
  int nx = NX, ny = NY, icnt = 0;
  double f[NY][NX], fn[NY][NX], dt,
    Lx = 1.0,  Ly = 1.0, kappa = KU, t = 0.0,
    u = 1.0, v = 1.0,
    dx = Lx/(double)(nx-2), dy = Ly/(double)(ny-2);
  FILE *fp;

  fp = fopen("advection.txt", "w");
  printf("NX:%d NY:%d\nk:%f mu:%f\n", nx, ny, kappa, mu);
  fprintf(fp,"%d %d\n%f %f\n", nx, ny, kappa, mu);

  initial(nx, ny, dx, dy, f);

  //CFL条件より
  dt = 0.2* MIN2(dx/fabs(u), dy/fabs(v));

  do{
    output(nx, ny, f, t, fp);
    advection(nx, ny, f, fn, u, v, dt, dx, dy);
    boundary(nx, ny, fn);
    update(nx, ny, f, fn);

    t += dt;

  } while (icnt++ < 9999 && t < 0.02 + dt);//t = 0.02 まで出力して欲しいから +dt をくっつけた

  fclose(fp);
  
  return 0;
}


//関数系の作成
double MIN2(double x, double y) {
  double min;

  if (x < y)
    min = x;
  else
    min = y;
  return min;
}

//流れの初期化
void   initial(int nx, int ny, double dx, double dy, double f[][nx]) {
  double  x,  y;

  //計算範囲は jx-1まで
  for(int jy=0 ; jy < ny; jy++) {
    for(int jx=0 ; jx < nx; jx++) {
      x = dx*(double)(jx - 1);
      y = dy*(double)(jy - 1);
      //あとで調整
      f[jy][jx] = sin(2.0*M_PI*x)*sin(2.0*M_PI*y);
    }
  }
  return;
}

//出力用
void output(int nx, int ny, double f[][nx], double t, FILE *fp) {
  //printf("t:%f\n", t);
  fprintf(fp,"%f\n", t);
  for(int jy = 0; jy < ny; jy++) {
    for(int jx = 0; jx < nx; jx++) {
      if(jx < nx-1){
        printf("%.2f ", f[jy][jx]);
        fprintf(fp, "%f ", f[jy][jx]);
      }
      else{
        printf("%.2f\n", f[jy][jx]);
        fprintf(fp, "%f\n", f[jy][jx]); //壁で折り返す
      }
    }
  }
}

//境界条件
void   boundary(int nx, int ny, double f[][nx]) {

  for(int jy=0 ; jy < ny; jy++) f[jy][0] = f[jy][nx-2];
  for(int jy=0 ; jy < ny; jy++) f[jy][nx-1] = f[jy][1];
  for(int jx=0 ; jx < nx; jx++) f[0][jx] = f[nx-2][jx];
  for(int jx=0 ; jx < nx; jx++) f[ny-1][jx] = f[1][jx];

  return;
}

//移流方程式の計算，一次精度風上差分法
void advection(int nx, int ny, double f[][nx], double fn[][nx], 
  double u, double v, double dt, double dx, double dy) {
  
  double cflx = u*dt/dx,  cfly = v*dt/dy;

  //jy=0, jy=ny-1は更新しない
  for(int jy = 1; jy < (ny - 1); jy++) {
    for(int jx = 1; jy < (nx - 1); jx++) {

      fn[jy][jx] = f[jy][jx];

      //  風上差分
      if(u > 0.0) 
        fn[jy][jx] += - cflx*(f[jy][jx] - f[jy][jx-1]);
      else        
        fn[jy][jx] += - cflx*(f[jy][jx+1] - f[jy][jx]);

      if(v > 0.0) 
        fn[jy][jx] += - cfly*(f[jy][jx] - f[jy-1][jx]);
      else        
        fn[jy][jx] += - cfly*(f[jy+1][jx] - f[jy][jx]);
    }
  }
  return;
}

//配列の更新
void update(int nx, int ny, double f[][nx], double fn[][ny]) {

  for(int jy = 0; jy < ny; jy++) {
    for(int jx = 0; jx < nx; jx++) {
      f[jy][jx] = fn[jy][jx];
    }
  }
  return;
}