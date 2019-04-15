/* CodePage: UTF-8 */
#include <stdlib.h>
#include <stdio.h>

typedef float T_REAL;





#if 0
#define R_STR_TO_UL wcstoul
#define R_STR_TO_LD wcstold
#else
#define R_STR_TO_UL strtoul
#define R_STR_TO_LD strtold
#endif

#define DF_REAL_OUT "%f"

// main_sci 128 1024 5 0.0 0.0 0.0 0.5 1.0
/*

szFNameTxt = "C:\a\Alpha7G-master\A01\out_data.txt";
szFNameBin = "C:\a\Alpha7G-master\A01\out_data.bin";
nTimeSet = 1000;
pF = mopen ( szFNameTxt, "rb" );
[ n, nGridSize, nTimeSteps, fTimeStep ] = mfscanf ( pF, "%u%u%f" );
[ n, fTempInitial, fTempBottom, fTempLeft, fTempRight, fTempTop ] = mfscanf ( pF, "%f%f%f%f%f" );
mclose ( pF );
pF = mopen ( szFNameBin, "rb" );
xData = mget( nGridSize*nGridSize*nTimeSteps, "f", pF );
nLine = 0:1/(nGridSize-1):1;
nTimeSetZ = nTimeSet*nGridSize*nGridSize;
mclose ( pF );
clf()
gcf().color_map = jetcolormap(64);
colorbar ( 0, 1 )
xMat = matrix ( xData(nTimeSetZ+1:nTimeSetZ+nGridSize*nGridSize), nGridSize, nGridSize );
Sgrayplot ( nLine, nLine, xMat, strf="041")
contour2d ( nLine, nLine, xMat, 10 );
*/

int main ( int argc, char const *argv[] ) {
    /*
        Init
    */
    unsigned nGridSize;
    unsigned nTimeSteps;
    T_REAL fGridStep;
    T_REAL fTimeStep;
    T_REAL fTempInitial;
    T_REAL fTempBottom;
    T_REAL fTempLeft;
    T_REAL fTempRight;
    T_REAL fTempTop;
    T_REAL fTime;
    T_REAL *pThis;
    {
        unsigned j=0;
        nGridSize = R_STR_TO_UL ( argv[++j], NULL, 0 );
        nTimeSteps = R_STR_TO_UL ( argv[++j], NULL, 0 );
        fTimeStep = R_STR_TO_LD ( argv[++j], NULL );
        fTempInitial = R_STR_TO_LD ( argv[++j], NULL );
        fTempBottom = R_STR_TO_LD ( argv[++j], NULL );
        fTempLeft = R_STR_TO_LD ( argv[++j], NULL );
        fTempRight = R_STR_TO_LD ( argv[++j], NULL );
        fTempTop = R_STR_TO_LD ( argv[++j], NULL );
        fGridStep = 1.0L / nGridSize;
    }
    unsigned nGridSizeWidth = nGridSize + 2;
    unsigned nGridSizeFull = nGridSizeWidth * nGridSizeWidth;
    pThis = (T_REAL*) malloc ( sizeof (T_REAL) * nGridSizeFull );
    /* Initial Temp */
    for ( unsigned i = 0; i < nGridSizeFull; ++i )
        pThis [i] = fTempInitial;
    /* Bottom Edge */
    for ( unsigned i = 0; i < nGridSizeWidth; ++i )
        pThis [i] = fTempBottom;
    /* Left Edge */
    for ( unsigned i = 0; i < nGridSizeWidth; ++i )
        pThis [i*nGridSizeWidth] = fTempLeft;
    /* Right Edge */
    for ( unsigned i = 0; i < nGridSizeWidth; ++i )
        pThis [i*nGridSizeWidth+nGridSizeWidth-1] = fTempRight;
    /* Top Edge */
    for ( unsigned i = 0; i < nGridSizeWidth; ++i )
        pThis [i+(nGridSizeWidth-1)*nGridSizeWidth] = fTempTop;
    /*
        Save Datas
    */
    FILE *pF = fopen ( "out_data.txt", "wb" );
    fprintf ( pF, "%u %u " DF_REAL_OUT "\n", nGridSize, nTimeSteps, fTimeStep );
    fprintf ( pF, DF_REAL_OUT " " DF_REAL_OUT " " DF_REAL_OUT " " DF_REAL_OUT " " DF_REAL_OUT "\n", fTempInitial, fTempBottom, fTempLeft, fTempRight, fTempTop );

    FILE *pFBin = fopen ( "out_data.bin", "wb" );
    /*
        T[ k - индекс по временному слою, i,j - индексы по пространственной сетке ]
        T[0,i,j] - начальные значения
        T[k,0,j] - левая грань
        T[k,Ni,j] - правая грань
        T[k,i,0] - нижняя грань
        T[k,i,Nj] - верхняя грань

     == Дискретный аналог исходного уравнения:
        ( T[k+1,*,*] - T[k,*,*] ) / t =
            ( T[*,i-1,j] - 2 T[*,i,j] + T[*,i+1,j] ) / h^2 +
            ( T[*,i,j-1] - 2 T[*,i,j] + T[*,i,j+1] ) / h^2

     == Неявная схема:
        ( T[k+1,i,j] - T[k,i,j] ) / t =
            ( T[k+1,i-1,j] - 2 T[k+1,i,j] + T[k+1,i+1,j] ) / h^2 +
            ( T[k+1,i,j-1] - 2 T[k+1,i,j] + T[k+1,i,j+1] ) / h^2
        >> изменим немного индекс k
        ( T[k,i,j] - T[k-1,i,j] ) / t =
            ( T[k,i-1,j] - 2 T[k,i,j] + T[k,i+1,j] ) / h^2 +
            ( T[k,i,j-1] - 2 T[k,i,j] + T[k,i,j+1] ) / h^2
        >> T[k,i,j] - расчитывыемый узел
        >> T[k-1,i,j] - расчитанное значения в узле в пред момент времени - константа
        >> Умножим обе части уравнения на t и h
        h^2 * ( T[k,i,j] - T[k-1,i,j] ) =
            t * ( T[k,i-1,j] - 2 T[k,i,j] + T[k,i+1,j] + T[k,i,j-1] - 2 T[k,i,j] + T[k,i,j+1] )
        >> Вынесем неизвестные части влево, а известные вправо
        h^2 T[k,i,j] + 2t T[k,i,j] + 2t T[k,i,j]
            -t * ( T[k,i-1,j] + T[k,i+1,j] + T[k,i,j-1] + T[k,i,j+1] )
            = h^2 T[k-1,i,j]
        >> Для наглядности сделаем замену
            T[k,i,j] == This
            T[k-1,i,j] == Old
            T[k,i-1,j] == Left
            T[k,i+1,j] == Right
            T[k,i,j-1] == Bottom
            T[k,i,j+1] == Top
        (h^2+4t)*This - t*(Left+Right+Bottom+Top) = h^2 * Old
        >> Считаем итерационнм методом, считая что нам известны:
            Left, Bottom - из пред расчётов или краевых условий
            Old - из пред временногослоя
            Bottom, Top - известны из пред итерации
        This = [h^2/(h^2+4t)] * Old + [t/(h^2+4t)] * (Left+Right+Bottom+Top)
        >> Заменим константы
            Const1 == [h^2/(h^2+4t)]
            Const2 == [t/(h^2+4t)]
        This = Const1 * Old + Const2 * (Left+Right+Bottom+Top)
    */

    T_REAL fConst1 = fGridStep*fGridStep/(fGridStep*fGridStep+((T_REAL)(4))*fTimeStep);
    T_REAL fConst2 = fTimeStep/(fGridStep*fGridStep+((T_REAL)(4))*fTimeStep);

    for ( unsigned k = 0; k < nTimeSteps; ++k ) {
        for ( unsigned i = 0; i < nGridSize * nGridSize; ++i) {
            const unsigned j = i+1+nGridSizeWidth+(2*(i/nGridSize));
            pThis[j] = ( fConst1 * pThis[j] ) + ( fConst2 * (
                pThis[j-1] + pThis[j+1] +
                pThis[j-nGridSizeWidth] +
                pThis[j+nGridSizeWidth] ) );
            fprintf ( pF, " " DF_REAL_OUT, pThis[j] );
        }
        fprintf ( pF, "\n\n" );
        printf("%u/%u\n", k+1, nTimeSteps );
        for ( unsigned k = 0; k < nGridSize; ++k ) {
            fwrite ( pThis+nGridSizeWidth+1+k*nGridSizeWidth, sizeof ( T_REAL ), nGridSize, pFBin );
        }
    }

    /*
        End
    */
    fclose ( pF );

    fclose ( pFBin );


    free ( pThis );

    return 0;
}
