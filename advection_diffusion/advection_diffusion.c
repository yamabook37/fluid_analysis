/*
更新者：山本
最終更新：2020/3/21 5:30
[問題点 (now fixed)]
・k=0.10, EPS=0.01の時，一致するデータが少ない
--> 条件：cnt++<9999 でその後の更新が止まってただけ
・未だ最適なEPSの値を見つかっていない
--> 1e-10 とかでok

[やりたいこと]
画像の名前を作るための部品をPythonに渡したい
例： KU = 0.10 で INITIAL_CONFIG = 3 なら"0.1" と "point" を渡す
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define NX (100+2)
#define NY (100+2)
#define KU (0.01) //k=1/Re
#define mu (0.10) //計算の安定性を決めるファクター mu > 2.5 だと計算が爆発する
// 時間合わせ用
#define EPS (1e-10)//「abs(a-b)<EPS」は「a==b」の言い換えだから小さければ小さいほど良い
#define DT (0.01) //表示させたい時間の間隔
#define ENDTIME (2.0) //ループの終了する時間


/******************************初期条件******************************/
// 初期条件の形が選べる（0:円 1:square 2:random 3:point）
#define INITIAL_CONFIG (1)
// 境界条件が選べる（0:periodic 1:fixed）
#define BOUNDARY_CONFIG (0)

// 写真を何データに一枚撮るかを決める。デフォルトは1(0 はダメ)
#define IMAGE_DEVIDE (20)


/*******************************************************************/
double MIN2(double x, double y);
//初期状態を決定
void initial(int nx, int ny, double dx, double dy, double f[][nx]);
//その時刻における 時刻と f[jy][jx] の値をファイルとターミナルにアウトプット
void output(int nx, int ny, double f[][nx], double t, FILE *data_fp);

//f[jy][jx] をもとにワンタイムステップ後の状態 fn[jy][jx] を計算
//fn は fn[0][jx] など（つまり境界）は更新されないことに注意
void advection(int nx, int ny, double f[][nx], double fn[][nx], double u, double v, double dt, double dx, double dy);

void x_advection(int nx, int ny, double f[][nx], double fn[][nx], double u, double dt, double dx);
void y_advection(int nx, int ny, double f[][nx], double fn[][nx], double v, double dt, double dy);
void diffusion(int nx, int ny, double f[][nx], double fn[][nx], double kappa, double dt, double dx, double dy);

//fn に境界条件を課す
void boundary(int nx, int ny, double fn[][nx]);
//更新。計算して求めた fn は新たな fn を求めるための f へと♪～
void update(int nx, int ny, double f[][nx], double fn[][nx]);


/*******************************************************************/
int main(){
  int nx = NX, ny = NY, icnt = 0, ocnt = 0;
  double f[NY][NX], fn[NY][NX], dt, t = 0.0,
    Lx = 1.0,  Ly = 1.0, kappa = KU, 
    u = 1.0, v = 1.0,
    dx = Lx/(double)(nx-2), dy = Ly/(double)(ny-2);
  FILE *data_fp, *picnum_fp;

  //時間合わせ
  int ti_cnt = 0;
  int ti_size = (int)((ENDTIME+0.01)/DT);
  double ti[ti_size];
  double ti_tmp;
  
  //実行時間計測用
  time_t start_t, end_t;
  start_t = time(NULL);

  // ランダム変数のシードは時刻から取る、つまり毎回違うシード
  srand((unsigned)time(NULL));

  data_fp = fopen("data/addif.txt", "w");
  picnum_fp = fopen("data/picture_number.txt", "w");

  printf("NX:%d NY:%d\nk:%f mu:%f\n", nx, ny, kappa, mu);
  fprintf(data_fp,"%d %d\n%f %f %f %f\n", nx-2, ny-2, Lx, Ly, kappa, mu);

  initial(nx, ny, dx, dy, f);
  //初期条件のfにも境界条件を課すのが正しい気がする。//1/21/20:18 mushano
  boundary(nx, ny, f);

  //CFL条件などより
  dt = MIN2(0.2* MIN2(dx/fabs(u), dy/fabs(v)), mu * MIN2(dx*dx,dy*dy)/kappa);
  printf("dt:%.10f\n",dt);
  
  //表示したい時間配列の用意
  /*
  for(ti_cnt = 0; ti_cnt<(int)((ENDTIME+0.01)/DT); ti_cnt ++) {
    ti[ti_cnt] = DT * ti_cnt;
    //printf("%d:DT=%f\n",ti_cnt, ti[ti_cnt]);
  }*/
  //初期化
  ti_cnt = 0;
  ti_tmp = 0.0;
  while(ti_tmp < ENDTIME + EPS){
    ti[ti_cnt] = ti_tmp;
    ti_tmp += DT;
    ti_cnt += 1;
  }

  // x方向に移流 -> y方向に移流 -> 拡散 の順に繰り返す
  // 逐一アップデートするのを忘れずに
  ti_cnt = 0;
  int arrayNum = sizeof ti / sizeof ti[0];
  do{
    if(ti_cnt < arrayNum-1 && t > ti[ti_cnt] - EPS) {
      //printf("%f\n",fabs(t - ti[ti_cnt]));
      output(nx, ny, f, t, data_fp);
      ti_cnt++;
      printf("t:%.10f\n", t);
    }

    x_advection(nx, ny, f, fn, u, dt, dx);
    boundary(nx, ny, fn);
    update(nx, ny, f, fn);

    y_advection(nx, ny, f, fn, v, dt, dy);
    boundary(nx, ny, fn);
    update(nx, ny, f, fn);

    diffusion(nx, ny, f, fn, kappa, dt, dx, dy);
    boundary(nx, ny, fn);
    update(nx, ny, f, fn);

    //printf("%.10f\n",t);
    t += dt;
    //icnt++;
  } while (/*icnt++ < 9999 &&*/ t < (ENDTIME + DT)); // 出力させたい t+0.01,
  // 参考までに dt~0.04(KU=0.01), dt~0.0004(KU=0.10)
  // dt=2.5e-4(KU=0.01) dt=2.5e-5(KU=0.10) dt=2.5e-6(KU=1.00)
  // icnt < 9999 を消去すればちゃんと欲しい枚数だけゲットできる、但し計算量は多くなる
  // --> NY=NX=202 なら実行時間：8[s](KU=0.01), 52[s](KU=0.1), 495[s](KU=1.0)

  //写真の枚数を出力するで～
  printf("number of pictures:%d\n", arrayNum);
  fprintf(picnum_fp,"%d", arrayNum);

  fclose(picnum_fp);
  fclose(data_fp);
  
  end_t = time(NULL);
  //time_t は秒単位までの計測
  printf("This calculatioin took %ld second \n", end_t-start_t);

  return 0;
}


/*******************************************************************/
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
  
  if(INITIAL_CONFIG == 0){
    //計算範囲は jx-1まで
    for(int jy=0 ; jy < ny; jy++) {
      for(int jx=0 ; jx < nx; jx++) {
        x = dx*(double)(jx - 1);
        y = dy*(double)(jy - 1);
        //あとで調整
        f[jy][jx] = sin(2.0*M_PI*x)*sin(2.0*M_PI*y);
      }
    }
  }

  if(INITIAL_CONFIG == 1){
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        if(0.3*nx < jx && jx < 0.7*nx && 0.3*ny < jy && jy < 0.7*ny){
          f[jy][jx] = 1.0;
        }
        else{
          f[jy][jx] = 0.0;
        }
      }
    }
  }

  if(INITIAL_CONFIG == 2){
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        f[jy][jx] = 0.0;
      }
    }
    for(int i = 0; i < ny*nx; i++) {
      f[rand()%NY][rand()%NX] = 1.0;
    }
  }

  if(INITIAL_CONFIG == 3){
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        f[jy][jx] = 0.0;
      }
    }
    f[ny/2][nx/2] = 1.0;
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

//移流方程式の計算，一次精度風上差分法 使いまわし
void advection(int nx, int ny, double f[][nx], double fn[][nx], 
  double u, double v, double dt, double dx, double dy) {
  
  double cflx = u*dt/dx,  cfly = v*dt/dy;

  //jy=0, jy=ny-1は更新しない -> ただの dummy だから
  for(int jy = 1; jy < (ny - 1); jy++) {
    for(int jx = 1; jx < (nx - 1); jx++) {

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

//一次元移流方程式の追加。yamamonmon の 2D のを援用
void x_advection(int nx, int ny, double f[][nx], double fn[][nx], double u, double dt, double dx){
  double cflx = u*dt/dx;

    //jy=0, jy=ny-1は更新しない
  for(int jy = 1; jy < (ny - 1); jy++) {
    for(int jx = 1; jx < (nx - 1); jx++) {

      fn[jy][jx] = f[jy][jx];

      //  風上差分
      if(u > 0.0) 
        fn[jy][jx] += - cflx*(f[jy][jx] - f[jy][jx-1]);
      else        
        fn[jy][jx] += - cflx*(f[jy][jx+1] - f[jy][jx]);
    }
  }
  return;
}

void y_advection(int nx, int ny, double f[][nx], double fn[][nx], double v, double dt, double dy){
  double cfly = v*dt/dy;

    //jy=0, jy=ny-1は更新しない
  for(int jy = 1; jy < (ny - 1); jy++) {
    for(int jx = 1; jx < (nx - 1); jx++) {

      fn[jy][jx] = f[jy][jx];

      //  風上差分
      if(v > 0.0) 
        fn[jy][jx] += - cfly*(f[jy][jx] - f[jy-1][jx]);
      else        
        fn[jy][jx] += - cfly*(f[jy+1][jx] - f[jy][jx]);
    }
  }
  return;
}


//拡散方程式は diffusion.c から使いまわし
void diffusion(int nx, int ny, double f[][nx], double fn[][nx], double kappa, double dt, double dx, double dy){
  //jy は通常 0 から ny-1 まで走るが、境界条件であるところのjy = 0, jy = ny-1 は更新しない。jxも同様
  for(int jy = 1; jy < ny-1; jy++) {
    for(int jx = 1; jx < nx-1; jx++) {
      fn[jy][jx] = f[jy][jx] + kappa * dt * 
      ( (f[jy][jx+1] - 2.0*f[jy][jx] + f[jy][jx-1])/dx/dx
        + (f[jy+1][jx] - 2.0*f[jy][jx] + f[jy-1][jx])/dy/dy
      );
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