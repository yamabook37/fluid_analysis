/*
最終更新：1/31 16:00
編集者：武者野
・周期境界条件のもとでの計算、つまり burgers の速度補正有りの作成途中（動いたがバグあり）
・物体境界はまだまだ
*/

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NX (200 + 3)
#define NY (100 + 3)
#define LX 2.0
#define LY 1.0
//#define KU (0.01)
#define mu (0.10) //0.1くらいにする必要がありそう
#define RE 1000   //レイノルズ数いじる //変域0.01<k<0.1 -> 10<Re<100

#define EPS (1e-10)   //「abs(a-b)<EPS」は「a==b」の言い換えだから小さければ小さいほど良い
#define DT (0.01)     //表示させたい時間の間隔，統一させるためになるべくこれで
#define ENDTIME (5.0) //ループの終了する時間，長めに取らないとデータ数少ない

//初期条件の形が選べる（0:円みたいの 1:ink 2:random 3:point）
#define INITIAL_CONFIG (0)
//境界条件が選べる（0:periodic 1:fixed 2:物体境界（調整中））
#define BOUNDARY_CONFIG (0)

//写真を何データに一枚撮るかを決める。デフォルトは1(0 はダメ)
#define IMAGE_DEVIDE (20)

#define CHECK_INTERVAL 100

//初期条件の形が選べる（0:円みたいの 1:ink 2:random 3:point）
#define INITIAL_CONFIG (0)
//境界条件が選べる（0:periodic 1:fixed 2:物体境界（作成中））
#define BOUNDARY_CONFIG (0)

double MIN2(double x, double y);

//初期状態を決定
void initial(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx]);
//その時刻における 時刻と f[jy][jx] の値をファイルとターミナルにアウトプット
void output(int nx, int ny, double f[][nx], double t, FILE *data_fp);

//f[jy][jx] をもとにワンタイムステップ後の状態 fn[jy][jx] を計算
//fn は fn[0][jx] など（つまり境界）は更新されないことに注意
void advection_diffusion(int nx, int ny, double f[][nx], double fn[][nx], double u[][nx], double v[][nx], double Re, double dt, double dx, double dy, int iw[][nx]);

void rotation(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx], double rot[][nx]);
void div_udt(int nx, int ny, double dt, double dx, double dy, double u[][nx], 
                              double v[][nx], double s[][nx], int iw[][nx]);

//un, vnに境界条件を課す
void boundary(int nx, int ny, double un[][nx], double vn[][nx]);
//更新。計算して求めた fn は新たな fn を求めるための f へと♪～
void update(int nx, int ny, double f[][nx], double fn[][nx], int iw[][nx]);
void set_wall(int nx,int ny,double dx,double dy,int iw[][nx],int ip[][nx]);

// SOR法で1ステップだけf更新。誤差|f - fn|の最大値を返す
double sor(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx], double omega, int ip[][nx]);
// sor をたくさん呼び出して poisson 方程式を解く。返り値は反復回数
int poisson2d(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx], double omega, double eps, int ip[][nx]);
// 残差の最大値を計算。反復中に呼び出すことで精度の上昇を見れる。
double residual(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx]);

void correction(int nx, int ny, double dt, double dx, double dy, double u[][nx], 
                              double v[][nx], double p[][nx], int iw[][nx]);

//ほぼ変更せず，流れに沿って作成
int main(){
  int nx = NX, ny = NY, icnt = 0, ocnt = 0;
  double f[NY][NX], fn[NY][NX], dt, it,
      Lx = LX, Ly = LY, kappa = 1 / RE, Re = RE, t = 0.0,
      u[NY][NX], un[NY][NX], v[NY][NX], vn[NY][NX],
      rot[NY][NX], div[NY][NX],
      dx = Lx / (double)(nx - 2), dy = Ly / (double)(ny - 2);
  int iw[NY][NX], ip[NY][NX];
  double s[NY][NX], p[NY][NX];
  FILE *data_fp, *picnum_fp;

  int ti_cnt = 0;
  int ti_size = (int)((ENDTIME + 0.01) / DT);
  double ti[ti_size];

  //実行時間計測用
  time_t start_t, end_t;
  start_t = time(NULL);

  // ランダム変数のシードは時刻から取る、つまり毎回違うシード
  srand((unsigned)time(NULL));

  data_fp = fopen("data/imcompressible.txt", "w");
  picnum_fp = fopen("data/picture_number.txt", "w");

  printf("NX:%d NY:%d\nRe:%f mu:%f\n", nx, ny, Re, mu);
  fprintf(data_fp, "%d %d\n%f %f\n", nx, ny, Re, mu);

  // 壁がどこにあるかを決定
  set_wall(nx,ny,dx,dy,iw,ip);

  initial(nx, ny, dx, dy, u, v);
  //初期条件のfにも境界条件を課すのが正しい気がする。//1/21/20:18 mushano
  //俺もそう思う //1/24/20:29 yamamoto
  boundary(nx, ny, u, v);

  dt = MIN2(0.2 * MIN2(dx, dy) / 1.0, 0.1 * MIN2(dx * dx, dy * dy) * Re);
  // 0.08くらい
  //printf("dt:%f\n",dt);

  //表示したい時間配列の用意
  for (ti_cnt = 0; ti_cnt < (int)((ENDTIME + 0.01) / DT); ti_cnt++){
    ti[ti_cnt] = DT * ti_cnt;
    //printf("%d:DT=%f\n",ti_cnt, ti[ti_cnt]);
  }
  //初期化
  ti_cnt = 0;
  

  // x方向に移流 -> y方向に移流 -> 拡散 の順に繰り返す。なんでこれで良いんだろう…
  // 逐一アップデートするのを忘れずに
  do{
    //IMAGE_DEVIDE回に一回だけoutputする。つまり写真を撮る回数を減らしてpythonの負担を軽くする。
    if (icnt % IMAGE_DEVIDE == 0){
      rotation(nx, ny, dx, dy, u, v, rot);
      //boundary(nx, ny, rot);
      //divergence(nx, ny, dx, dy, u, v, div);
      //boundary(nx, ny, div);
      output(nx, ny, rot, t, data_fp); //複数個用意したいね
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

    advection_diffusion(nx, ny, u, un, u, v, Re, dt, dx, dy, iw); //u
    advection_diffusion(nx, ny, v, vn, u, v, Re, dt, dx, dy, iw); //v
    boundary(nx, ny, un, vn);
    update(nx, ny, u, un, iw);
    update(nx, ny, v, vn, iw);

    div_udt(nx,ny,dt,dx,dy,u,v,s,iw);

    it = poisson2d(nx, ny, dx, dy, p, s, 1.8, 1.0e-06, ip);
    printf("it:%d\n", it);
    correction(nx,ny,dt,dx,dy,u,v,p,iw);

    t += dt;

    icnt++;

  } while (/*icnt++ < 9999 &&*/ t < (ENDTIME + 0.01)); // 出力させたい t+0.01, dt~0.04

  //写真の枚数を出力するで～
  printf("number of pictures:%d\n", ocnt);
  fprintf(picnum_fp, "%d", ocnt);

  fclose(picnum_fp);
  fclose(data_fp);

  end_t = time(NULL);
  printf("This calculatioin took %ld second \n", end_t - start_t);

  return 0;
}

//関数系の作成
double MIN2(double x, double y){
  double min;

  if (x < y)
    min = x;
  else
    min = y;
  return min;
}

void set_wall(int nx,int ny,double dx,double dy,int iw[][nx],int ip[][nx]){
  if(BOUNDARY_CONFIG == 2){
    for (int jy = 0; jy < ny; jy++){
      for (int jx = 0; jx < nx; jx++){
        //iwの決定
        if (1 < jy && jy < ny - 2){
          if (1 < jx && jx < nx - 2)
            iw[jy][jx] = 0;
          if (jx >= nx - 2)
            iw[jy][jx] = 2;
        }
        else{
          iw[jy][jx] = 1;
        }
        //ipの決定 1 <= jy の等号に注意
        if (1 <= jy && jy < ny - 2){
          if (1 <= jx && jx < nx - 2) ip[jy][jx] = 0;
          if (jx >= nx - 2) ip[jy][jx] = 2;
        }
        else{
          ip[jy][jx] = 1;
        }
      }
    }
  }
  else{
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        iw[jy][jx] = ip[jy][jx] = 0;
      }
    }
  }
  
}

//流れの初期化
//0:円みたいの 1:ink 2:random 3:point
void initial(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx]){
  double x, y, kx = 2.0 * M_PI, ky = 2.0 * M_PI;

  //circle
  if (INITIAL_CONFIG == 0){
    //計算範囲は jx-1まで
    for (int jy = 0; jy < ny; jy++){
      for (int jx = 0; jx < nx; jx++){
        x = dx * (double)(jx - 1);
        y = dy * (double)(jy - 1);
        //あとで調整
        u[jy][jx] = -cos(2.0 * M_PI * x) * sin(2.0 * M_PI * y) / kx;
        v[jy][jx] = sin(2.0 * M_PI * x) * cos(2.0 * M_PI * y) / ky;
      }
    }
  }

  // inkの流れる実験, uだけ
  if (INITIAL_CONFIG == 1){
    for (int jy = 0; jy < ny; jy++){
      for (int jx = 0; jx < nx; jx++){
        u[jy][jx] = 0.0;
        v[jy][jx] = 0.0;
      }
    }
    u[ny / 2][2] = 1.0;
  }

  //random
  if (INITIAL_CONFIG == 2){
    for (int jy = 0; jy < ny; jy++){
      for (int jx = 0; jx < nx; jx++){
        u[jy][jx] = 0.0;
        v[jy][jx] = 0.0;
      }
    }
    for (int i = 0; i < ny * nx; i++)
    {
      u[rand() % NY][rand() % NX] = 1.0;
      v[rand() % NY][rand() % NX] = 1.0;
    }
  }

  // point
  if (INITIAL_CONFIG == 3){
    for (int jy = 0; jy < ny; jy++){
      for (int jx = 0; jx < nx; jx++){
        u[jy][jx] = 0.0;
        v[jy][jx] = 0.0;
      }
    }
    u[ny / 2][nx / 2] = 1.0;
    v[ny / 2][nx / 2] = 1.0;
  }

  return;
}

//出力用
void output(int nx, int ny, double f[][nx], double t, FILE *data_fp){
  //printf("t:%f\n", t);
  fprintf(data_fp, "%f\n", t);
  for (int jy = 0; jy < ny; jy++){
    for (int jx = 0; jx < nx; jx++){
      if (jx < nx - 1){
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
void boundary(int nx, int ny, double un[][nx], double vn[][nx]){
  //周期
  if(BOUNDARY_CONFIG == 0){
    for(int jy=0 ; jy < ny; jy++) un[jy][0] = un[jy][nx-2];
    for(int jy=0 ; jy < ny; jy++) un[jy][nx-1] = un[jy][1];
    for(int jx=0 ; jx < nx; jx++) un[0][jx] = un[ny-2][jx];
    for(int jx=0 ; jx < nx; jx++) un[ny-1][jx] = un[1][jx];

    for(int jy=0 ; jy < ny; jy++) vn[jy][0] = vn[jy][nx-2];
    for(int jy=0 ; jy < ny; jy++) vn[jy][nx-1] = vn[jy][1];
    for(int jx=0 ; jx < nx; jx++) vn[0][jx] = vn[ny-2][jx];
    for(int jx=0 ; jx < nx; jx++) vn[ny-1][jx] = vn[1][jx];
  }
  //固定
  if(BOUNDARY_CONFIG == 1){
    for(int jy=0 ; jy < ny; jy++) un[jy][0] = 0.0;
    for(int jy=0 ; jy < ny; jy++) un[jy][nx-1] = 0.0;
    for(int jx=0 ; jx < nx; jx++) un[0][jx] = 0.0;
    for(int jx=0 ; jx < nx; jx++) un[ny-1][jx] = 0.0;

    for(int jy=0 ; jy < ny; jy++) vn[jy][0] = 0.0;
    for(int jy=0 ; jy < ny; jy++) vn[jy][nx-1] = 0.0;
    for(int jx=0 ; jx < nx; jx++) vn[0][jx] = 0.0;
    for(int jx=0 ; jx < nx; jx++) vn[ny-1][jx] = 0.0;
  }

  // 物体境界
  if(BOUNDARY_CONFIG == 2){
    //流入境界
    for (int jx = 0; jx < nx; jx++){
      un[1][jx] = un[ny - 2][jx] = 1;
      vn[1][jx] = vn[ny - 2][jx] = 0;
    }
    for (int jy = 0; jy < ny; jy++){
      un[jy][1] = 1;
      vn[jy][1] = 0;
    }

    //流出境界
    for (int jy = 2; jy < ny - 2; jy++){
      un[jy][nx - 3] = un[jy][nx - 2];
      vn[jy][nx - 3] = vn[jy][nx - 2];
    }
  }
  
  

  return;
}

//cfl の配列化，関数をまとめた
void advection_diffusion(int nx, int ny, double f[][nx], double fn[][nx], double u[][nx], double v[][nx],
                         double Re, double dt, double dx, double dy, int iw[][nx]){

  for (int jy = 0; jy < ny; jy++){
    for (int jx = 0; jx < nx; jx++){
      //計算領域でなければ更新しない
      if(iw[jy][jx] != 0) continue;
      fn[jy][jx] = f[jy][jx];

      //移流
      if (u[jy][jx] > 0.0){
        fn[jy][jx] += -u[jy][jx] * (f[jy][jx] - f[jy][jx - 1]) / dx * dt;
      }
      else{
        fn[jy][jx] += -u[jy][jx] * (f[jy][jx + 1] - f[jy][jx]) / dx * dt;
      }

      if (v[jy][jx] > 0.0){
        fn[jy][jx] += -v[jy][jx] * (f[jy][jx] - f[jy - 1][jx]) / dy * dt;
      }
      else{
        fn[jy][jx] += -v[jy][jx] * (f[jy + 1][jx] - f[jy][jx]) / dy * dt;
      }

      //拡散, kappaからReに変更
      fn[jy][jx] += dt *
                    ((f[jy][jx + 1] - 2.0 * f[jy][jx] + f[jy][jx - 1]) / (dx * dx) + (f[jy + 1][jx] - 2.0 * f[jy][jx] + f[jy - 1][jx]) / (dy * dy)) / Re;
    }
  }
}

void rotation(int nx, int ny, double dx, double dy, double u[][nx], double v[][nx], double rot[][nx]){
  for (int jy = 1; jy < ny - 1; jy++){
    for (int jx = 1; jx < nx - 1; jx++){
      rot[jy][jx] = 0.5 * (v[jy + 1][jx + 1] - v[jy + 1][jx] + v[jy][jx + 1] - v[jy][jx]) / dx - 0.5 * (u[jy + 1][jx + 1] - u[jy][jx + 1] + u[jy + 1][jx] - u[jy][jx]) / dy;
    }
  }
  return;
}

void div_udt(int nx, int ny, double dx, double dy, double dt, double u[][nx], double v[][nx], double s[][nx], int iw[][nx]){
  for (int jy = 1; jy < ny - 1; jy++){
    for (int jx = 1; jx < nx - 1; jx++){
      if(iw[jy][jx] != 0) continue;
      s[jy][jx] = 0.5 * (u[jy + 1][jx + 1] - u[jy + 1][jx] + u[jy][jx + 1] - u[jy][jx]) / dx 
                + 0.5 * (v[jy + 1][jx + 1] - v[jy][jx + 1] + v[jy + 1][jx] - v[jy][jx]) / dy;
      s[jy][jx] /= dt;
    }
  }
}


double sor(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx], double omega, int ip[][nx]){
  double fn, err = 0.0;
  for (int jy = 1; jy < ny - 1; jy++){
    for (int jx = 1; jx < nx - 1; jx++){
      if(ip[jy][jx] != 0) continue;

      fn = ((f[jy][jx + 1] + f[jy][jx - 1]) / (dx * dx) 
         + (f[jy + 1][jx] + f[jy - 1][jx]) / (dy * dy) - s[jy][jx]) 
         * 0.5 * dx * dx * dy * dy / (dx * dx + dy * dy);
      err = fmax(fabs(fn - f[jy][jx]), err);
      f[jy][jx] = (1.0 - omega) * f[jy][jx] + omega * fn;
    }
  }

  return err;
}

int poisson2d(int nx, int ny, double dx, double dy, 
      double f[][nx], double s[][nx], double omega, double eps, int ip[][nx]){
  int icnt = 0, imax = 99999;
  double err;

  // 反復
  while(icnt++ < imax){
    //fの更新と更新前後の差の最大値確認
    err = sor(nx,ny,dx,dy,f,s,omega,ip);
    //更新してもほとんど変わらないようなら終了
    //printf("err:%.10f\n", err);
    if(eps > err) return icnt;
    if(icnt%CHECK_INTERVAL == 0){
      double resi = residual(nx, ny, dx, dy, f, s);
      //printf("reidual:%f\n", resi);
    }
    // printf("err:%f\n", err);
  }

  // imax 回反復しても収束しないなら-1を返して終了
  return -1;
}

double residual(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx]){
  double res, rmax = 0.0;
  //各格子点の(d^2f/dx^2 + d^2f/dy^2 - s)を計算
  for (int jy = 1; jy < ny - 1; jy++){
    for (int jx = 1; jx < nx - 1; jx++){
      res = (f[jy][jx + 1] - 2.0 * f[jy][jx] + f[jy][jx - 1]) / (dx * dx) 
          + (f[jy + 1][jx] - 2.0 * f[jy][jx] + f[jy - 1][jx]) / (dy * dy)
          - s[jy][jx];
      rmax = fmax(res, rmax);
    }
  }

  return rmax;
}

void correction(int nx, int ny, double dt, double dx, double dy, double u[][nx], 
                              double v[][nx], double p[][nx], int iw[][nx]){
  for (int jx = 2; jx < ny - 2; jx++){
    for (int jx = 2; jx < nx - 2; jx++){
      if (iw[jx][jx] > 0)continue;

      u[jx][jx] += -0.5 * (p[jx][jx] - p[jx][jx - 1]
                 + p[jx - 1][jx] - p[jx - 1][jx - 1]) / dx * dt;
      v[jx][jx] += -0.5 * (p[jx][jx] - p[jx - 1][jx]
                 + p[jx][jx - 1] - p[jx - 1][jx - 1]) / dy * dt;
    }
  }
}

//配列の更新
void update(int nx, int ny, double f[][nx], double fn[][ny], int iw[][nx]){

  for (int jy = 0; jy < ny; jy++){
    for (int jx = 0; jx < nx; jx++){
      //計算領域じゃなかったら更新しない
      if(iw[jy][jx] != 0) continue;

      f[jy][jx] = fn[jy][jx];
    }
  }
  return;
}