#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define  NX  256+1
#define  NY  256+1
// SOR 法を何回反復したら残差を check するか
#define CHECK_INTERVAL 100

// SOR法で1ステップだけf更新。誤差|f - fn|の最大値を返す
double sor(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx], double omega);
// sor をたくさん呼び出して poisson 方程式を解く。返り値は反復回数
int poisson2d(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx], double omega, double eps);
// 残差の最大値を計算。反復中に呼び出すことで精度の上昇を見れる。
double residual(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx]);
// 計算で get したfを解析解と比較
double error_f(int nx, int ny, double dx, double dy, double f[][nx]);
//解くべきs(x,y)の関数形
double s_func(double x, double y);
//上のs(x,y)の元での解析式f(x,y)
double f_analitic(double x, double y);

int main(int argc, char *argv[]){
  int nx = NX, ny = NY, it;
  double s[NY][NX], f[NY][NX];
  double Lx = 1.0, Ly = 1.0, x, y;
  double dx = Lx/(double)(nx-1), dy = Ly/(double)(ny-1);

  // 配列の初期化
  for(int jy = 0; jy < ny; jy++){
    for(int jx = 0; jx < nx; jx++){
      // mesh -> 座標 に変換
      x = dx*(double)jx; y = dy*(double)jy;
      f[jy][jx] = 0.0;
      s[jy][jx] = s_func(x,y);
    }
  }

  // for(int jy = 0; jy < ny; jy++) {
  //   for(int jx = 0; jx < nx; jx++) {
  //     if(jx < nx-1) printf("%f ", s[jy][jx]);
  //     else printf("%f\n", s[jy][jx]);
  //   }
  // }

  it = poisson2d(nx,ny,dx,dy,f,s,1.8,1.0e-8);
  printf("\n%3dx%3d error=%9.3e iteration=%d\n",nx-1,ny-1,
            error_f(nx,ny,dx,dy,f),it);
  
  // for(int jy = 0; jy < ny; jy++) {
  //   for(int jx = 0; jx < nx; jx++) {
  //     if(jx < nx-1) printf("%f ", f[jy][jx]);
  //     else printf("%f\n", f[jy][jx]);
  //   }
  // }
  // printf("%d\n");
  // for(int jy = 0; jy < ny; jy++) {
  //   for(int jx = 0; jx < nx; jx++) {
  //     x = dx*(double)jx; y = dy*(double)jy;
  //     if(jx < nx-1) printf("%f ", f_analitic(y,x));
  //     else printf("%f\n", f_analitic(y,x));
  //   }
  // }

  return 0;
}

#define KX 2.0*M_PI
#define KY 2.0*M_PI

double s_func(double x, double y){
  return -(KX*KX + KY*KY)*f_analitic(x, y);
}

double f_analitic(double x, double y){
  return sin(KX*x)*sin(KY*y);
}

double sor(int nx, int ny, double dx, double dy, double f[][nx], double s[][nx], double omega){
  double fn, err = 0.0;
  for (int jy = 1; jy < ny - 1; jy++){
    for (int jx = 1; jx < nx - 1; jx++){
      fn = ((f[jy][jx + 1] + f[jy][jx - 1]) / (dx * dx) 
         + (f[jy + 1][jx] + f[jy - 1][jx]) / (dy * dy) - s[jy][jx]) 
         * 0.5 * dx * dx * dy * dy / (dx * dx + dy * dy);
      err = fmax(fabs(fn - f[jy][jx]), err);
      f[jy][jx] = (1.0 - omega) * f[jy][jx] + omega * fn;
    }
  }

  return err;
}

int poisson2d(int nx, int ny, double dx, double dy, double f[][nx], 
                            double s[][nx], double omega, double eps){
  int icnt = 0, imax = 99999;
  double err;

  // 反復
  while(icnt++ < imax){
    //fの更新と差の確認
    err = sor(nx,ny,dx,dy,f,s,omega);
    if(eps > err) return icnt;
    if(icnt%CHECK_INTERVAL == 0){
      double resi = residual(nx, ny, dx, dy, f, s);
      printf("reidual:%f\n", resi);
    }
    // printf("err:%f\n", err);
  }

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

double error_f(int nx, int ny, double dx, double dy, double f[][nx]){
  double x, y, err = 0.0;

  for (int jy = 1; jy < ny - 1; jy++){
    for (int jx = 1; jx < nx - 1; jx++){
      //座標に変換
      x = dx * (double)jx;
      y = dy * (double)jy;

      err += fabs(f[jy][jx] - f_analitic(x,y));
    }
  }

  //err を格子点の数で平均
  return err / (double)((nx - 2) * (ny - 2));
}
