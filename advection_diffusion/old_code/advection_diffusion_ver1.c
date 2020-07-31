/*
最終更新：1/24 5:00
編集者：武者野
・fの値を新たなディレクトリ"data"に出力するようにした
・写真の枚数を別ファイル"picture_number"に出力
・icntとは別にoutputした回数を数える変数 ocnt を追加
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NX (200+2)
#define NY (200+2)
#define KU (0.01)
#define mu (0.10)//0.1くらいにする必要がありそう

//初期条件の形が選べる（0:円みたいの 1:square 2:random 3:point）
#define INITIAL_CONFIG (1)
//境界条件が選べる（0:periodic 1:fixed）
#define BOUNDARY_CONFIG (0)

//写真を何データに一枚撮るかを決める。デフォルトは1(0 はダメ)
#define IMAGE_DEVIDE (20)
/*
毎回出力させていたらデータ多すぎて今の python コードだとアニメが描けない。
メッシュが 50*50 だったら 5/KU くらいで決めると写真がちょうど1000枚くらいになって軽い（軽いのか？）
*/

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


//ほぼ変更せず，流れに沿って作成
int main(){
  int nx = NX, ny = NY, icnt = 0, ocnt = 0;
  double f[NY][NX], fn[NY][NX], dt,
    Lx = 1.0,  Ly = 1.0, kappa = KU, t = 0.0,
    u = 1.0, v = 1.0,
    dx = Lx/(double)(nx-2), dy = Ly/(double)(ny-2);
  FILE *data_fp, *picnum_fp;

  // ランダム変数のシードは時刻から取る、つまり毎回違うシード
  srand((unsigned)time(NULL));

  data_fp = fopen("data/advection_diffusion.txt", "w");
  picnum_fp = fopen("data/picture_number.txt", "w");

  printf("NX:%d NY:%d\nk:%f mu:%f\n", nx, ny, kappa, mu);
  fprintf(data_fp,"%d %d\n%f %f\n", nx, ny, kappa, mu);

  initial(nx, ny, dx, dy, f);
  //初期条件のfにも境界条件を課すのが正しい気がする。//1/21/20:18 mushano
  boundary(nx, ny, f);

  //CFL条件などより
  dt = MIN2(0.2* MIN2(dx/fabs(u), dy/fabs(v)), mu * MIN2(dx*dx,dy*dy)/kappa );
  //printf("dt:%f\n",dt);

  // x方向に移流 -> y方向に移流 -> 拡散 の順に繰り返す。なんでこれで良いんだろう…
  // 逐一アップデートするのを忘れずに
  do{

    //IMAGE_DEVIDE回に一回だけoutputする。つまり写真を撮る回数を減らしてpythonの負担を軽くする。
    if(icnt%IMAGE_DEVIDE == 0){
      output(nx, ny, f, t, data_fp);
      ocnt++;
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

    t += dt;

  } while (icnt++ < 9999 && t < 1.01); // 出力させたい t+0.01, dt~0.04

  //写真の枚数を出力するで～
  printf("number of pictures:%d\n", ocnt);
  fprintf(picnum_fp,"%d", ocnt);

  fclose(picnum_fp);
  fclose(data_fp);
  
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