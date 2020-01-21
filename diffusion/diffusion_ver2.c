#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/*
最終更新：1/16 14:00
・ランダムの時の初期化の仕方がおかしかったから訂正
・mu を#defineにした
・mu を出力するようにした
・つまりpythonの方もmuを受け取れるように変えた
*/

// NX = NY = 11 くらいにして動かすとターミナルに出てくる数値眺めるだけでも
// 拡散していく様子が見れて面白いから是非！
#define NX (5+1)
#define NY (5+1)
#define KU (1.0)
#define mu (0.20)//計算の安定性を決めるファクター mu > 2.5 だと計算が爆発する

// 1:初期条件をランダムに生成 0:固定
#define CONFIG_RANDOM (1)

//初期状態を決定
void initial(int nx, int ny, double f[][nx]);
//その時刻における f[jy][jx] の値をファイルとターミナルにアウトプット
void output(int nx, int ny, double f[][nx], double t, FILE *fp);
//f[jy][jx] をもとにワンタイムステップ後の状態 fn[jy][jx] を計算。
//fn は fn[0][jx] など（つまり境界）は更新されないことに注意
void diffusion(int nx, int ny, double f[][nx], double fn[][nx], double kappa, double dt, double dx, double dy);
//fn に境界条件を課す
void boundary(int nx, int ny, double fn[][nx]);
//更新。計算して求めた fn は新たな fn を求めるための f へと♪～
void update(int nx, int ny, double f[][nx], double fn[][nx]);


int main(){
  int nx = NX, ny = NY, icnt = 0;
  double f[NY][NX], fn[NY][NX], dt,
         Lx = 1.0,  Ly = 1.0, kappa = KU, t = 0.0,
         dx = Lx/(double)(nx-1), dy = Ly/(double)(ny-1);
  FILE *fp;

  // ランダム変数のシードは時刻から取る、つまり毎回違うシード
  srand((unsigned)time(NULL));

  fp = fopen("diffusion.txt", "w");
  //printf("NX:%d NY:%d\nk:%f mu:%f\n", nx, ny, kappa, mu);
  fprintf(fp,"%d %d\n%f %f\n", nx, ny, kappa, mu);

  initial(nx, ny, f);

  dt = mu * fmin(dx*dx,dy*dy)/kappa;

  do{
    output(nx, ny, f, t, fp);
    diffusion(nx, ny, f, fn, kappa, dt, dx, dy);
    boundary(nx, ny, fn);
    update(nx, ny, f, fn);

    t += dt;

  } while (icnt++ < 9999 && t < 0.02 + dt);//t = 0.02 まで出力して欲しいから +dt をくっつけた

  fclose(fp);
  return 0;
}

void initial(int nx, int ny, double f[][nx]){
  if(CONFIG_RANDOM == 1){
    //ひとまず0を入れる
    for(int jy = 0; jy < ny; jy++) for(int jx = 0; jx < nx; jx++) {
      f[jy][jx] = 0.0;
    }
    //全部の点の10%くらいをランダムに選んで大きい値を持たせる
    for(int i = 0; i < NX*NY/10; i++) {
      f[rand()%NY][rand()%NX] = 1.0;
    }
  }
  else{
    for(int jy = 0; jy < ny; jy++) {
      for(int jx = 0; jx < nx; jx++) {
        if(0.3*nx < jx && jx < 0.7*nx && 0.3*ny < jy && jy < 0.7*ny){
          f[jy][jx] = 1.0;
        }
        // else if(0.8*nx < jx && jx < 0.9*nx && 0.8*ny < jy && jy < 0.9*ny){
        //   f[jy][jx] = 5.0;
        // }
        else{
          f[jy][jx] = 0.0;
        }
      }
    }  
  }
  
  //check用
  // for(int jy = 0; jy < ny; jy++) {
  //   for(int jx = 0; jx < nx; jx++) {
  //     if(jx < nx-1){
  //       printf("%.1f ", f[jy][jx]);
  //     }
  //     else{
  //       printf("%.1f\n", f[jy][jx]);
  //     }
  //   }
  // }
  return;
}

void output(int nx, int ny, double f[][nx], double t, FILE *fp){
  //printf("t:%f\n", t);
  fprintf(fp,"%f\n", t);
  for(int jy = 0; jy < ny; jy++) {
    for(int jx = 0; jx < nx; jx++) {
      if(jx < nx-1){
        //printf("%.2f ", f[jy][jx]);
        fprintf(fp, "%f ", f[jy][jx]);
      }
      else{
        //printf("%.2f\n", f[jy][jx]);
        fprintf(fp, "%f\n", f[jy][jx]);
      }
    }
  }
}

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

void boundary(int nx, int ny, double fn[][nx]){
  for(int jy=0 ; jy < ny; jy++) fn[jy][0] = 0.0;
  for(int jy=0 ; jy < ny; jy++) fn[jy][nx-1] = 0.0;
  for(int jx=0 ; jx < nx; jx++) fn[0][jx] = 0.0;
  for(int jx=0 ; jx < nx; jx++) fn[ny-1][jx] = 0.0;
  //check 用
  // printf("boundary chech\n");
  // for(int jy = 0; jy < ny; jy++) {
  //   for(int jx = 0; jx < nx; jx++) {
  //     if(jx < nx-1){
  //       printf("%.1f ", f[jy][jx]);
  //     }
  //     else{
  //       printf("%.1f\n", f[jy][jx]);
  //     }
  //   }
  // }
  return;
}

void update(int nx, int ny, double f[][nx], double fn[][ny]){
  //範囲に注目。
  for(int jy = 0; jy < ny; jy++) {
    for(int jx = 0; jx < nx; jx++){
      f[jy][jx] = fn[jy][jx];
    }
  }
  return;
}