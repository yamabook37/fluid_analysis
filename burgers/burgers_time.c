/*
最終更新：1/31 8:00
編集者：山本
[変更点]
・kappaの逆数のReを定義した
・advection(x,y)+diffusion をまとめた関数を作成
[改善点]
・初期条件の吟味
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NX (50+2)
#define NY (50+2)
#define LX 1.0
#define LY 1.0
//#define KU (0.01)
#define mu (0.10)//0.1くらいにする必要がありそう
#define RE 2000 //レイノルズ数いじる //変域0.01<k<0.1 -> 10<Re<100
/* 
試して欲しいパラメータ@土肥くん
Re=10,point <-> ad_ifのk=0.1,point
Re=100,point <-> ad_ifのk=0.01,point
  検証：回転と発散の処理の有無
*/

#define EPS (1e-10)//「abs(a-b)<EPS」は「a==b」の言い換えだから小さければ小さいほど良い
#define DT (0.01) //表示させたい時間の間隔，統一させるためになるべくこれで
#define ENDTIME (5.0) //ループの終了する時間，長めに取らないとデータ数少ない

//初期条件の形が選べる（0:円みたいの 1:ink 2:random 3:point）
#define INITIAL_CONFIG (2)
//境界条件が選べる（0:periodic 1:fixed）
#define BOUNDARY_CONFIG (0)

//写真を何データに一枚撮るかを決める。デフォルトは1(0 はダメ)
#define IMAGE_DEVIDE (20)


double MIN2(double x, double y);

//初期状態を決定
void initial(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx]);
//その時刻における 時刻と f[jy][jx] の値をファイルとターミナルにアウトプット
void output(int nx, int ny, double f[][nx], double t, FILE *data_fp);

//f[jy][jx] をもとにワンタイムステップ後の状態 fn[jy][jx] を計算
//fn は fn[0][jx] など（つまり境界）は更新されないことに注意
void advection_diffusion(int nx, int ny, double f[][nx], double fn[][nx], double u[][nx], double v[][nx], double Re, double dt, double dx ,double dy);

void rotation(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx], double rot[][nx]);
void divergence(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx], double div[][nx]);


//fn に境界条件を課す
void boundary(int nx, int ny, double fn[][nx]);
//更新。計算して求めた fn は新たな fn を求めるための f へと♪～
void update(int nx, int ny, double f[][nx], double fn[][nx]);


//ほぼ変更せず，流れに沿って作成
int main(){
  int nx = NX, ny = NY, icnt = 0, ocnt = 0;
  double f[NY][NX], fn[NY][NX], dt,
    Lx = LX,  Ly = LY, kappa = 1/RE, Re = RE, t = 0.0,
    u[NY][NX], un[NY][NX], v[NY][NX], vn[NY][NX],
    rot[NY][NX],  div[NY][NX],
    dx = Lx/(double)(nx-2), dy = Ly/(double)(ny-2);
  FILE *data_fp, *picnum_fp;

  int ti_cnt = 0;
  int ti_size = (int)((ENDTIME+0.01)/DT);
  double ti[ti_size];

  //実行時間計測用
  time_t start_t, end_t;
  start_t = time(NULL);

  // ランダム変数のシードは時刻から取る、つまり毎回違うシード
  srand((unsigned)time(NULL));

  data_fp = fopen("data/burgers_time.txt", "w");
  picnum_fp = fopen("data/picture_number.txt", "w");

  printf("NX:%d NY:%d\nRe:%f mu:%f\n", nx, ny, Re, mu);
  fprintf(data_fp,"%d %d\n%f %f\n", nx, ny, Re, mu);

  initial(nx, ny, dx, dy, u, v);
  //初期条件のfにも境界条件を課すのが正しい気がする。//1/21/20:18 mushano
  //俺もそう思う //1/24/20:29 yamamoto
  boundary(nx, ny, u);  boundary(nx, ny, v);  

  dt = MIN2(0.2*MIN2(dx,dy)/1.0, 0.1*MIN2(dx*dx,dy*dy)*Re);
  // 0.08くらい
  //printf("dt:%f\n",dt);

  //表示したい時間配列の用意
  for(ti_cnt = 0; ti_cnt<(int)((ENDTIME+0.01)/DT); ti_cnt ++) {
    ti[ti_cnt] = DT * ti_cnt;
    //printf("%d:DT=%f\n",ti_cnt, ti[ti_cnt]);
  }
  //初期化
  ti_cnt = 0;

  // x方向に移流 -> y方向に移流 -> 拡散 の順に繰り返す。なんでこれで良いんだろう…
  // 逐一アップデートするのを忘れずに
  do{
    
    //IMAGE_DEVIDE回に一回だけoutputする。つまり写真を撮る回数を減らしてpythonの負担を軽くする。
    if(icnt%IMAGE_DEVIDE == 0){
      rotation(nx, ny, dx, dy, u, v, rot);  boundary(nx,ny, rot);
      divergence(nx, ny, dx, dy, u, v, div);  boundary(nx,ny, div);
      output(nx, ny, u, t, data_fp); //複数個用意したいね
      ocnt++;
    }
/*
    if(fabs(t - ti[ti_cnt]) < EPS){
      rotation(nx, ny, dx, dy, u, v, rot);  boundary(nx,ny, rot);
      divergence(nx, ny, dx, dy, u, v, div);  boundary(nx,ny, div);
      output(nx, ny, rot, t, data_fp); //複数個用意したいね
      ocnt++;
      ti_cnt++;
      printf("t:%.10f\n", t);
    } */

    advection_diffusion(nx,ny,u,un,u,v,Re,dt,dx,dy); //u
    advection_diffusion(nx,ny,v,vn,u,v,Re,dt,dx,dy); //v
    boundary(nx,ny,un);    boundary(nx,ny,vn);
    update(nx,ny,u,un);    update(nx,ny,v,vn);

    t += dt;
    icnt++;

  } while (/*icnt++ < 9999 &&*/ t < (ENDTIME+0.01)); // 出力させたい t+0.01, dt~0.04

  //写真の枚数を出力するで～
  printf("number of pictures:%d\n", ocnt);
  fprintf(picnum_fp,"%d", ocnt);

  fclose(picnum_fp);
  fclose(data_fp);

  end_t = time(NULL);
  printf("This calculatioin took %ld second \n", end_t - start_t);
  
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
//0:円みたいの 1:ink 2:random 3:point
void  initial(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx]) {
  double x, y, kx = 2.0*M_PI, ky = 2.0*M_PI;
  
  //circle
  if(INITIAL_CONFIG == 0){
    //計算範囲は jx-1まで
    for(int jy=0 ; jy < ny; jy++) {
      for(int jx=0 ; jx < nx; jx++) {
        x = dx*(double)(jx - 1);
        y = dy*(double)(jy - 1);
        //あとで調整
        u[jy][jx] = -cos(2.0*M_PI*x)*sin(2.0*M_PI*y) /kx;
        v[jy][jx] = sin(2.0*M_PI*x)*cos(2.0*M_PI*y) /ky;
      }
    }
  }

  // inkの流れる実験, uだけ
  if(INITIAL_CONFIG == 1){
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        u[jy][jx] = 0.0;
        v[jy][jx] = 0.0;
      }
    }
    u[ny/2][2] = 1.0;
  }

  //random
  if(INITIAL_CONFIG == 2){
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        u[jy][jx] = 0.0;
        v[jy][jx] = 0.0;
      }
    }
    for(int i = 0; i < ny*nx; i++) {
      u[rand()%NY][rand()%NX] = 1.0;
      v[rand()%NY][rand()%NX] = 1.0;
    }
  }

  // point
  if(INITIAL_CONFIG == 3){
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        u[jy][jx] = 0.0;
        v[jy][jx] = 0.0;
      }
    }
    u[ny/2][nx/2] = 1.0;
    v[ny/2][nx/2] = 1.0;
  }
  
  return;
}

//出力用
void output(int nx, int ny, double f[][nx], double t, FILE *data_fp) {
  //printf("t:%f\n", t);
  fprintf(data_fp,"%f\n", t);
  for(int jy = 0; jy < ny; jy++) {
    for(int jx = 0; jx < nx; jx++) {
      if(jx < nx-1){
        //printf("%.2f ", f[jy][jx]);
        fprintf(data_fp, "%f ", f[jy][jx]);
      }
      else{
        //printf("%.2f\n", f[jy][jx]);
        fprintf(data_fp, "%f\n", f[jy][jx]); //壁で折り返す
      }
    }
  }
}

//境界条件
void boundary(int nx, int ny, double f[][nx]) {
  if(BOUNDARY_CONFIG == 0){
    for(int jy=0 ; jy < ny; jy++) f[jy][0] = f[jy][nx-2];
    for(int jy=0 ; jy < ny; jy++) f[jy][nx-1] = f[jy][1];
    for(int jx=0 ; jx < nx; jx++) f[0][jx] = f[ny-2][jx];
    for(int jx=0 ; jx < nx; jx++) f[ny-1][jx] = f[1][jx];
  }

  if(BOUNDARY_CONFIG == 1){
    for(int jy=0 ; jy < ny; jy++) f[jy][0] = 0.0;
    for(int jy=0 ; jy < ny; jy++) f[jy][nx-1] = 0.0;
    for(int jx=0 ; jx < nx; jx++) f[0][jx] = 0.0;
    for(int jx=0 ; jx < nx; jx++) f[ny-1][jx] = 0.0;
  }

  return;
}

//cfl の配列化，関数をまとめた
void advection_diffusion(int nx, int ny, double f[][nx], double fn[][nx], double u[][nx], double v[][nx], 
    double Re, double dt, double dx ,double dy){
    
  for(int jy = 1; jy < ny - 1; jy++) {
    for(int jx = 1; jx < nx - 1; jx++) {

      fn[jy][jx] = f[jy][jx];

      //移流
      if(u[jy][jx] > 0.0) {
        fn[jy][jx] += - u[jy][jx]*(f[jy][jx] - f[jy][jx-1])/dx*dt;
      }
      else {
        fn[jy][jx] += - u[jy][jx]*(f[jy][jx+1] - f[jy][jx])/dx*dt;
      }

      if(v[jy][jx] > 0.0) {
        fn[jy][jx] += - v[jy][jx]*(f[jy][jx] - f[jy-1][jx])/dy*dt;
      }
      else {
        fn[jy][jx] += - v[jy][jx]*(f[jy+1][jx] - f[jy][jx])/dy*dt;
      }

      //拡散, kappaからReに変更
      fn[jy][jx] += dt*
                  (
                    (f[jy][jx+1] - 2.0*f[jy][jx] + f[jy][jx-1])/(dx*dx)
                    + (f[jy+1][jx] - 2.0*f[jy][jx] + f[jy-1][jx])/(dy*dy)
                  ) /Re;
    }
  }
}

void rotation(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx], double rot[][nx]) {
  for(int jy=1 ; jy < ny-1; jy++) {
    for(int jx=1 ; jx < nx-1; jx++) {
      rot[jy][jx] = 0.5*(v[jy+1][jx+1] - v[jy+1][jx]
                    + v[jy][jx+1] - v[jy][jx])/dx
                  - 0.5*(u[jy+1][jx+1] - u[jy][jx+1]
                    + u[jy+1][jx] - u[jy][jx])/dy;
    }
  }
  return;
}

void divergence(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx], double div[][nx]) {
  for(int jy=1 ; jy < ny-1; jy++) {
    for(int jx=1 ; jx < nx-1; jx++) {
      div[jy][jx] = 0.5*(u[jy+1][jx+1] - u[jy+1][jx]
                    + u[jy][jx+1] - u[jy][jx])/dx
                  + 0.5*(v[jy+1][jx+1] - v[jy][jx+1]
                    + v[jy+1][jx] - v[jy][jx])/dy;
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