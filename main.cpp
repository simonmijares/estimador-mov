#include "estimador.h"

using namespace cv;
using namespace std;

double getPSNR ( const Mat& I1, const Mat& I2);
cudaError_t Cudainiciador(int dispositivo);

int main()
{
	int metodo=NINGUNO;
	ofstream myfile;
	timeval tiempo1, tiempo2;
	//double seconds_per_count;
	ostringstream fpsTexto;
	float fps=0;
	double contador;
	estimador esti(2);
	int tacumulado;
	int tcontador=0;
	int tmuestras=0;
	int tmuestrasacumuladas=0;
	bool condSalida;
	bool tipografica=false;
	bool cuda=false;
	int   contPSNR;
	Point textp;
	int PsnrGrafica[128];
	int tiempo[128];
	char tecla;
	contPSNR=0;
	contador=0;
	condSalida=false;
	textp.x=50;
	textp.y=50;
	Cudainiciador(0);
	//QueryPerformanceFrequency(&freq);
	//seconds_per_count = 1 / static_cast<double>(freq.LowPart);
	for (int i=0; i<=127;i++){
		tiempo[i]=0;
		PsnrGrafica[i]=0;
	}
	namedWindow("diferencia",1);
	namedWindow("100*PSNR",1);
	esti.capturar(cuda);
	waitKey(33);
	//	QueryPerformanceCounter(&tiempo1);
	//tiempo1=clock();
	gettimeofday(&tiempo1,NULL);
	while (!condSalida){
	// Procedimiento via clásica.
//		waitKey(300);
		if (cuda)
			esti.CalcularPSNRCuda();
		else
			esti.CalcularPSNR();
//		esti.CalcularPSNRCuda();
		esti.estimar(metodo);
		if (cuda)
		putText( esti.error, "tpf: "+fpsTexto.str()+" CUDA", textp, 4, 1, Scalar(255,255,255), 2, 8);
		else
		{
			putText( esti.error, "tpf: "+fpsTexto.str(), textp, 4, 1, Scalar(255,255,255), 2, 8);
		}
		if (!tipografica)
			imshow("diferencia",esti.error);
		else
			imshow("diferencia",esti.Buff1);

		contador++;
		tecla=cvWaitKey(16);
		esti.capturar(cuda);
		PsnrGrafica[contPSNR]=(int)floor(100*esti.psnrV);
		CvPlot::clear("100*PSNR");
		CvPlot::plot("100*PSNR", PsnrGrafica , 128);
/*	if (tcontador>200){
		QueryPerformanceCounter((LARGE_INTEGER *)&tiempo2);
		sec=((float)(tiempo2-tiempo1)/(float)freq);
		fps=200/sec;
		//cout<<"fps: "<<sec<<" ";
		//fpsTexto.clear();
		//fpsTexto.str("");
		//fpsTexto<<fixed<<setprecision(3)<<fps;
		cout<<fixed<<setprecision(3)<<fps<<" ";
		QueryPerformanceCounter((LARGE_INTEGER *)&tiempo1);
		tcontador=0;
	}*/
		if (tcontador>16){
			int tics;
			double secs;
			//QueryPerformanceCounter(&tiempo2);
			gettimeofday(&tiempo2,NULL);
			tiempo[tmuestras]=(tiempo2.tv_usec - tiempo1.tv_usec) / 1000.0;   // us to ms
			//(tiempo2.LowPart-tiempo1.LowPart)>>4;
			
			tacumulado=0;
			if (tmuestras<32){
				for (int i=0; i<=32 ; i++) {
					tacumulado+=tiempo[i];
				}
			}
			else {
				for (int i=tmuestras-32; i<=tmuestras ; i++) {
					tacumulado+=tiempo[i];
				}

			}

				secs=tacumulado>>5;
				//	secs=1000*(double)tics*seconds_per_count;
			//fps=(float)tmuestrasacumuladas*100*(float)freq/(float)tacumulado;
			cout<<"tpf: "<<secs<<" ";
			cout << setiosflags(ios::fixed) << setprecision(3) << esti.psnrV << "dB\n";
			fpsTexto.clear();
			fpsTexto.str("");
			fpsTexto<<fixed<<setprecision(3)<<secs;
			tcontador=0;
			//QueryPerformanceCounter(&tiempo1);
			gettimeofday(&tiempo1,NULL);
			tmuestras%=127;
			tmuestras++;
		}
		switch (tecla)
		{
		case 'e':
			tipografica=!tipografica;
			break;
		case 'x':
			condSalida=true;
			break;
		case 's':
			cout<<"Exportando Resultados...\n";
			myfile.open ("Resultados.txt");
			myfile << "Resultados:\nPSNR:\n";
			for (int i=0; i<128 ; i++) {
				myfile << PsnrGrafica[i]<<", ";//"Writing this to a file.\n";
			}
				myfile<<"\nTiempo:\n";
				for (int i=0; i<128 ; i++) {
				myfile << 1000*tiempo[i]<<", ";//"Writing this to a file.\n";
			}

			myfile.close();
			break;
		case 'p':
			waitKey();
			break;
		case 'c':
			cuda=!cuda;
			break;
		default:
			break;
		}
		contPSNR++;
		contPSNR%=127;
		tcontador++;
	}
    // Add vectors in parallel.
    //cudaError_t cudaStatus = absdiffCuda(c, a, b, SizeX,SizeY);
/*  if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addWithCuda failed!");
        return 1;
    }*/

	//getchar();
    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
/*  cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
        return 1;
    }
	*/
	CvPlot::clear("100*PSNR");
	destroyAllWindows();
    return 0;
}

/*Mat	absdiff( Mat a, Mat b)
{
	Mat salida;
	salida=a-b;

		return salida;
}*/

/*double getPSNR(const Mat& I1, const Mat& I2)
{
    Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);        // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    if( sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double mse  = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}*/

cudaError_t Cudainiciador(int dispositivo){

	   cudaDeviceProp  prop;

    int count;
    cudaGetDeviceCount( &count );
    for (int i=0; i< count; i++) {
        cudaGetDeviceProperties( &prop, i );
        printf( "   --- General Information for device %d ---\n", i );
        printf( "Name:  %s\n", prop.name );
        printf( "Compute capability:  %d.%d\n", prop.major, prop.minor );
        printf( "Clock rate:  %d\n", prop.clockRate );
        printf( "Device copy overlap:  " );
        if (prop.deviceOverlap)
            printf( "Enabled\n" );
        else
            printf( "Disabled\n");
        printf( "Kernel execution timeout :  " );
        if (prop.kernelExecTimeoutEnabled)
            printf( "Enabled\n" );
        else
            printf( "Disabled\n" );

        printf( "   --- Memory Information for device %d ---\n", i );
        printf( "Total global mem:  %ld\n", prop.totalGlobalMem );
        printf( "Total constant Mem:  %ld\n", prop.totalConstMem );
        printf( "Max mem pitch:  %ld\n", prop.memPitch );
        printf( "Texture Alignment:  %ld\n", prop.textureAlignment );

        printf( "   --- MP Information for device %d ---\n", i );
        printf( "Multiprocessor count:  %d\n",
                    prop.multiProcessorCount );
        printf( "Shared mem per mp:  %ld\n", prop.sharedMemPerBlock );
        printf( "Registers per mp:  %d\n", prop.regsPerBlock );
        printf( "Threads in warp:  %d\n", prop.warpSize );
        printf( "Max threads per block:  %d\n",
                    prop.maxThreadsPerBlock );
        printf( "Max thread dimensions:  (%d, %d, %d)\n",
                    prop.maxThreadsDim[0], prop.maxThreadsDim[1],
                    prop.maxThreadsDim[2] );
        printf( "Max grid dimensions:  (%d, %d, %d)\n",
                    prop.maxGridSize[0], prop.maxGridSize[1],
                    prop.maxGridSize[2] );
        printf( "\n" );
    }
	    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system.
		cudaStatus = cudaSetDevice(dispositivo);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
 //       goto Error;
    }
	else {
		printf("Cuda iniciada correctamente");  
	}
	return cudaStatus;

}
