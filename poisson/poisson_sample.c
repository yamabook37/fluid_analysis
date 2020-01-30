#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pgm.h"

#define  NX  256+1
#define  NY  256+1

//http://warp.da.ndl.go.jp/info:ndljp/pid/3481431/www.eto.titech.ac.jp/contents/sub03/chapter07.html

// 使わないと思われる式とか値とか変えてないところもあります
// int で定義してるものをforのなかでまた定義してしまってるものあります、使わない方消してください
//　s：ポアソン方程式の右辺項
// 詳しい説明は参考資料見たほうがいいです

int  main(int argc, char *argv[])
{
     int       nx = NX,    ny = NY,    jx,   jy,  it;
     double    s[NY][NX],  f[NY][NX];
     double    Lx = 1.0,   Ly = 1.0,   x,   y,
               dx = Lx/(double)(nx-1), dy = Ly/(double)(ny-1);

     for(int jy = 0; j < ny; jy++) {
         for(int jx = 0; i < nx; jx++) {
             x = dx*(double)jx;   y = dy*(double)jy;
             f[jy][jx] = 0.0;
             s[jy][jx] = _s(x,y);
         }
     }
     it = poisson2d(nx,ny,dx,dy,f[][nx],s[][nx],1.8,1.0e-8);
     printf("\n%3dx%3d error=%9.3e iteration=%d\n",nx-1,ny-1,
            error_f(nx,ny,dx,dy,f[][nx]),it);

     return 0;
}

//バーガースで↓は定義してないみたいです
#define  KX  2.0*M_PI
#define  KY  2.0*M_PI


double  _g(double x, double y) { return sin(KX*x)*sin(KY*y); }
double  _s(double x, double y) { return -(KX*KX + KY*KY)*_g(x,y); }



double   sor
// ====================================================================
// purpos     :  SOR

(/
   int      nx,            /* grid number in the x-direction         */
   int      ny,            /* grid number in the y-direction         */
   double   dx,            /* grid spacing in the x-direction        */
   double   dy,            /* grid spacing in the y-direction        */
   double   f[][nx],       /* dependent variable                     */
   double   s[][nx],       /* source term                            */
   double   omega          /* relaxation parameter                   */
)
//　ポアソン方程式をSOR法で解いた式がf[jy][jx]
//　jy,jxなど変えてますがミスってるところもあるかも
//  omega：加算（緩和）係数であり、1.8付近で最も早く反復計算が収束するらしい
//        1<omega<2で使用する
{
     int      jy,    jx;
     double   fn,   err = 0.0;

     for(int jy = 1; j < ny-1; jy++) {
         for(int jx = 1; i < nx-1; jx++) {

             fn = (  (f[jy][jx+1] + f[jy][jx-1])/(dx*dx)
                   + (f[jy+1][jx] + f[jy-1][jx])/(dy*dy) - s[jy][jx] )
                *0.5*dx*dx*dy*dy/(dx*dx + dy*dy);

             err = MAX2(fabs(fn - f[jy][jx]), err);

             f[jy][jx] += (1.0 - omega)*f[jy][jx] + omega*fn;
         }
     }

     return err;
}



double   residual
// ====================================================================
// purpos     :  residual of Poisson equation
(
   int      nx,            /* grid number in the x-direction         */
   int      ny,            /* grid number in the y-direction         */
   double   dx,            /* grid spacing in the x-direction        */
   double   dy,            /* grid spacing in the y-direction        */
   double   f[][nx],       /* dependent variable                     */
   double   s[][nx]        /* source term                            */
)
// --------------------------------------------------------------------
//  反復計算の過程で残差の最大値
//　反復計算の50回ごとに出力すると減少しているのがわかる？
{

     double   res,  rmax = 0.0;

     for(int jy = 1; jy < ny-1; jy++) {
         for(int jx = 1; jx < nx-1; jx++) {
             res = (f[jy][jx+1] - 2.0*f[jy][jx] + f[jy][jx-1])/(dx*dx)
                 + (f[jy+1][jx] - 2.0*f[jy][jx] + f[jy-1][jx])/(dy*dy)
                 - s[jy][jx];

             rmax = MAX2(res, rmax);
         }
     }

     return rmax;
}


int   poisson2d
// ====================================================================
// purpos     :  SOR法でポアソン方程式を解いていく    //
(
   int      nx,            /* grid number in the x-direction         */
   int      ny,            /* grid number in the y-direction         */
   double   dx,            /* grid spacing in the x-direction        */
   double   dy,            /* grid spacing in the y-direction        */
   double   f[][nx],       /* dependent variable                     */
   double   s[][nx],       /* source term                            */
   double   omega,         /* relaxation parameter                   */
   double   eps            /* convergenc threshold                   */

// return   iteretion number
)
// --------------------------------------------------------------------
{
     int      icnt = 0,   imax = 99999;
     double   err;

     while(icnt++ < imax) {
           if(eps > (err = sor(nx,ny,dx,dy,f[][nx],s[][nx],omega)))
          　return icnt;
#ifdef DEBUG
//         if(icnt % 50 == 0) printf("convergence=[%5d]=%10.4e\n",
//                                   icnt, err);
           if(icnt % 50 == 0) printf("conv=[%5d]=%10.4e res=%10.4e\n",
                                     icnt, err,
                                     residual(nx,ny,dx,dy,f[][nx],s[][nx]));
#endif
     }
     return icnt;
}


double  error_f
// ====================================================================

// purpos     :  エラーの精査
(
   int      nx,            /* grid number in the x-direction         */
   int      ny,            /* grid number in the y-direction         */
   double   dx,            /* grid spacing in the x-direction        */
   double   dy,            /* grid spacing in the y-direction        */
   double   f[][nx]        /* dependent variable                     */
)
// --------------------------------------------------------------------
{
     double   x,    y,    err = 0.0;

     for(int jy=1 ; jy < ny-1; jy++) {
         for(int jx=1 ; jx < nx-1; jx++) {
             x = dx*(double)jx;
             y = dy*(double)jy;
             err += fabs(f[jy][jx] - _g(x, y));
         }
     }

     return err/(double)((nx -2)*(ny - 2));
}
