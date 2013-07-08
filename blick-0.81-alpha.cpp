//=========================================================================================================================================
//      Blick - Open Source Gaze Tracker
//
//      by Arturo Bianchetti
//
//      Developed using OpenCV and cvblob (Cristobal Carnero Li\~nan)
//
//      Capabilities:   - gaze tracking.
//                      - pupil diameter measurement.
//                      - video rendering.
//
//      License: You can download, use, modify and distribute this software under the terms of
//      GNU GPL license.
//
//      Citation: if you want to cite this software use the following.
//
//      @MastersThesis{bianchetti,
//      author = {Bianchetti, Arturo},
//      title = {Determinaci\'on del di\'ametro pupilar ocular en tiempo real},
//      school = {Grupo de \'Optica y' y Visi\'on, Facultad de Ingenier\'ia, Universidad de Buenos Aires},
//      year = {2012}
//      }
//
//=========================================================================================================================================

#include <iomanip>
#if (defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || (defined(__APPLE__) & defined(__MACH__)))
#include <cv.h>
#include <highgui.h>
#else
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif
#include <cvblob.h>
#include <math.h>
#include <sys/time.h>
#define GNUPLOT "gnuplot -persist"
#include <unistd.h>
#include <ipp.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace cvb;
using namespace std;

// Varibles to be set in the future using .xml file
int pupilThreshold = 10;
int purkinjeThreshold = 235;
int dataRecord = 0;
int minArea = 2000;
int maxArea = 15000;
int minPurkinjeArea = 10;
int maxPurkinjeArea = 200;
char buf [64];
int pupilCircularity = 90;
int homographicCalibration = 0;
int startEyetracker = 0;
int plotPupilDiameter = 0;
int mmToPixelCalibration = 0;
int movePointer=0;
int tagFrames=0;
double mmToPixelRatio = 98;

// For trackbars
void PupilThreshold(int id){id;}
void PurkinjeThreshold (int id){id;}
void DataRecord(int id){id;}
void MinArea (int id){id;}
void MaxArea (int id){id;}
void MinPkjArea (int id){id;}
void MaxPkjArea (int id){id;}
void PupilCircularity (int id){id;}
void TrackingCalibration (int id){id;}
void TrackingEye (int id){id;}
void PlotGnuplot (int id){id;}
void Pixelmm (int id){id;}

// Time returned in seconds.microseconds (not working without ipp)
double GetMicroSeconds(void)
{
    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    double microSecondsTime = (double) timeStamp.tv_sec + (double) 1e-6 * timeStamp.tv_usec;
    return(microSecondsTime);
}

// Move mouse pointer
void MouseMove(int x, int y)
{
    Display *displayMain = XOpenDisplay(NULL);

    if(displayMain == NULL)
    {
        fprintf(stderr, "Display open error!\n");
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(displayMain);
    XWarpPointer(displayMain, None, root, 0, 0, 0, 0, x, y);
    XCloseDisplay(displayMain);
}

// Function to display screen to input subject data. I know this is crappy but it works!.
char* DataEnter(const char* windowText, const char* screenText)
{
    static char concatData[100];
    static char tempData[100];
    int keyHit = 0;
    int loop=1;
    double cursorTimer = clock();
    char cursorChar[64];

    cvNamedWindow(windowText);
    cvMoveWindow(windowText,0,0);
    IplImage *blackScreen = cvCreateImage(cvSize(640,240),8,3);

    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX_SMALL, .7, .7, 0, 1, CV_AA);

    concatData[0]='\0';
        while( loop ) {
                cvMoveWindow(windowText,401,641);
                cvZero(blackScreen);
                cvPutText(blackScreen, screenText, cvPoint(10, 50), &font, CV_RGB(0,255,0));
                keyHit = (cvWaitKey( 1 )&0xff);
                switch (keyHit) {
                    case 13: if (strlen(concatData) >= 1)concatData[strlen(concatData)-1]='\0';
                        break;
                    case 10: loop = 0;
                        break;
                    case 32: strcpy(tempData,concatData);strcat(concatData," ");
                        break;
                    case 33: strcpy(tempData,concatData);strcat(concatData,"!");
                        break;
                    case 34: strcpy(tempData,concatData);strcat(concatData,"\"");
                        break;
                    case 35: strcpy(tempData,concatData);strcat(concatData,"#");
                        break;
                    case 36: strcpy(tempData,concatData);strcat(concatData,"$");
                        break;
                    case 37: strcpy(tempData,concatData);strcat(concatData,"%");
                        break;
                    case 38: strcpy(tempData,concatData);strcat(concatData,"&");
                        break;
                    case 39: strcpy(tempData,concatData);strcat(concatData,"'");
                        break;
                    case 40: strcpy(tempData,concatData);strcat(concatData,"(");
                        break;
                    case 41: strcpy(tempData,concatData);strcat(concatData,")");
                        break;
                    case 42: strcpy(tempData,concatData);strcat(concatData,"*");
                        break;
                    case 43: strcpy(tempData,concatData);strcat(concatData,"+");
                        break;
                    case 44: strcpy(tempData,concatData);strcat(concatData,",");
                        break;
                    case 45: strcpy(tempData,concatData);strcat(concatData,"-");
                        break;
                    case 46: strcpy(tempData,concatData);strcat(concatData,".");
                        break;
                    case 47: strcpy(tempData,concatData);strcat(concatData,"/");
                        break;
                    case 48: strcpy(tempData,concatData);strcat(concatData,"0");
                        break;
                    case 49: strcpy(tempData,concatData);strcat(concatData,"1");
                        break;
                    case 50: strcpy(tempData,concatData);strcat(concatData,"2");
                        break;
                    case 51: strcpy(tempData,concatData);strcat(concatData,"3");
                        break;
                    case 52: strcpy(tempData,concatData);strcat(concatData,"4");
                        break;
                    case 53: strcpy(tempData,concatData);strcat(concatData,"5");
                        break;
                    case 54: strcpy(tempData,concatData);strcat(concatData,"6");
                        break;
                    case 55: strcpy(tempData,concatData);strcat(concatData,"7");
                        break;
                    case 56: strcpy(tempData,concatData);strcat(concatData,"8");
                        break;
                    case 57: strcpy(tempData,concatData);strcat(concatData,"9");
                        break;
                    case 58: strcpy(tempData,concatData);strcat(concatData,":");
                        break;
                    case 59: strcpy(tempData,concatData);strcat(concatData,";");
                        break;
                    case 60: strcpy(tempData,concatData);strcat(concatData,"<");
                        break;
                    case 61: strcpy(tempData,concatData);strcat(concatData,"=");
                        break;
                    case 62: strcpy(tempData,concatData);strcat(concatData,">");
                        break;
                    case 63: strcpy(tempData,concatData);strcat(concatData,"?");
                        break;
                    case 64: strcpy(tempData,concatData);strcat(concatData,"@");
                        break;
                    case 65: strcpy(tempData,concatData);strcat(concatData,"A");
                        break;
                    case 66: strcpy(tempData,concatData);strcat(concatData,"B");
                        break;
                    case 67: strcpy(tempData,concatData);strcat(concatData,"C");
                        break;
                    case 68: strcpy(tempData,concatData);strcat(concatData,"D");
                        break;
                    case 69: strcpy(tempData,concatData);strcat(concatData,"E");
                        break;
                    case 70: strcpy(tempData,concatData);strcat(concatData,"F");
                        break;
                    case 71: strcpy(tempData,concatData);strcat(concatData,"G");
                        break;
                    case 72: strcpy(tempData,concatData);strcat(concatData,"H");
                        break;
                    case 73: strcpy(tempData,concatData);strcat(concatData,"I");
                        break;
                    case 74: strcpy(tempData,concatData);strcat(concatData,"J");
                        break;
                    case 75: strcpy(tempData,concatData);strcat(concatData,"K");
                        break;
                    case 76: strcpy(tempData,concatData);strcat(concatData,"L");
                        break;
                    case 77: strcpy(tempData,concatData);strcat(concatData,"M");
                        break;
                    case 78: strcpy(tempData,concatData);strcat(concatData,"N");
                        break;
                    case 79: strcpy(tempData,concatData);strcat(concatData,"O");
                        break;
                    case 80: strcpy(tempData,concatData);strcat(concatData,"P");
                        break;
                    case 81: strcpy(tempData,concatData);strcat(concatData,"Q");
                        break;
                    case 82: strcpy(tempData,concatData);strcat(concatData,"R");
                        break;
                    case 83: strcpy(tempData,concatData);strcat(concatData,"S");
                        break;
                    case 84: strcpy(tempData,concatData);strcat(concatData,"T");
                        break;
                    case 85: strcpy(tempData,concatData);strcat(concatData,"U");
                        break;
                    case 86: strcpy(tempData,concatData);strcat(concatData,"V");
                        break;
                    case 87: strcpy(tempData,concatData);strcat(concatData,"W");
                        break;
                    case 88: strcpy(tempData,concatData);strcat(concatData,"X");
                        break;
                    case 89: strcpy(tempData,concatData);strcat(concatData,"Y");
                        break;
                    case 90: strcpy(tempData,concatData);strcat(concatData,"Z");
                        break;
                    case 91: strcpy(tempData,concatData);strcat(concatData,"[");
                        break;
                    case 92: strcpy(tempData,concatData);strcat(concatData,"\\");
                        break;
                    case 93: strcpy(tempData,concatData);strcat(concatData,"]");
                        break;
                    case 94: strcpy(tempData,concatData);strcat(concatData,"^");
                        break;
                    case 95: strcpy(tempData,concatData);strcat(concatData,"_");
                        break;
                    case 96: strcpy(tempData,concatData);strcat(concatData,"`");
                        break;
                    case 97: strcpy(tempData,concatData);strcat(concatData,"a");
                        break;
                    case 98: strcpy(tempData,concatData);strcat(concatData,"b");
                        break;
                    case 99: strcpy(tempData,concatData);strcat(concatData,"c");
                        break;
                    case 100: strcpy(tempData,concatData);strcat(concatData,"d");
                        break;
                    case 101: strcpy(tempData,concatData);strcat(concatData,"e");
                        break;
                    case 102: strcpy(tempData,concatData);strcat(concatData,"f");
                        break;
                    case 103: strcpy(tempData,concatData);strcat(concatData,"g");
                        break;
                    case 104: strcpy(tempData,concatData);strcat(concatData,"h");
                        break;
                    case 105: strcpy(tempData,concatData);strcat(concatData,"i");
                        break;
                    case 106: strcpy(tempData,concatData);strcat(concatData,"j");
                        break;
                    case 107: strcpy(tempData,concatData);strcat(concatData,"k");
                        break;
                    case 108: strcpy(tempData,concatData);strcat(concatData,"l");
                        break;
                    case 109: strcpy(tempData,concatData);strcat(concatData,"m");
                        break;
                    case 110: strcpy(tempData,concatData);strcat(concatData,"n");
                        break;
                    case 111: strcpy(tempData,concatData);strcat(concatData,"o");
                        break;
                    case 112: strcpy(tempData,concatData);strcat(concatData,"p");
                        break;
                    case 113: strcpy(tempData,concatData);strcat(concatData,"q");
                        break;
                    case 114: strcpy(tempData,concatData);strcat(concatData,"r");
                        break;
                    case 115: strcpy(tempData,concatData);strcat(concatData,"s");
                        break;
                    case 116: strcpy(tempData,concatData);strcat(concatData,"t");
                        break;
                    case 117: strcpy(tempData,concatData);strcat(concatData,"u");
                        break;
                    case 118: strcpy(tempData,concatData);strcat(concatData,"v");
                        break;
                    case 119: strcpy(tempData,concatData);strcat(concatData,"w");
                        break;
                    case 120: strcpy(tempData,concatData);strcat(concatData,"x");
                        break;
                    case 121: strcpy(tempData,concatData);strcat(concatData,"y");
                        break;
                    case 122: strcpy(tempData,concatData);strcat(concatData,"z");
                        break;
                    case 123: strcpy(tempData,concatData);strcat(concatData,"{");
                        break;
                    case 124: strcpy(tempData,concatData);strcat(concatData,"|");
                        break;
                    case 125: strcpy(tempData,concatData);strcat(concatData,"}");
                        break;
                    case 126: strcpy(tempData,concatData);strcat(concatData,"~");
                        break;
                    case 209: strcpy(tempData,concatData);strcat(concatData,"Ñ");
                        break;
                    case 241: strcpy(tempData,concatData);strcat(concatData,"ñ");
                        break;
                    }
            if (clock() - cursorTimer < 300000){sprintf (cursorChar,  "%s|", concatData);}
            if (clock() - cursorTimer > 300000){sprintf (cursorChar,  "%s", concatData);}
            if (clock() - cursorTimer > 600000){cursorTimer=clock();}
            cvPutText(blackScreen, cursorChar, cvPoint(10, 100), &font, CV_RGB(0,255,0));
            cvNamedWindow(windowText);
            cvShowImage(windowText,blackScreen);
            }
        return concatData;
        cvDestroyWindow(windowText);
}

// Main function
int main( int argc, char** argv )
{

   // Image for menu background
    IplImage *settings = cvCreateImage(cvSize(400,1),8,1);
    cvZero(settings);
    cvAddS(settings,cvScalar(200),settings);
    cvNamedWindow("Settings");
    cvShowImage("Settings",settings);
    cvMoveWindow("Settings",0,0);

    //Image for pupil tracking
    cvNamedWindow("Pupil tracking");
    cvResizeWindow("Pupil tracking",640,480);
    cvMoveWindow("Pupil tracking",401,0);

    // Define font
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 1, CV_AA);

    // frame counter 1
    int frameCounter = 0;

    // frame counter 2
    int gnuplotFrameCounter = 0;

    double plotTime = 0;

    // Chars for path to gnuplot and local path
    char path[256];
    char gnuplotPath[256];

    // Variables for data & settings
    char subjectName[100];
    char subjectAge[100];
    char subjectComments[100];

    char settingsPlot[100];
    char settingsFoldername[100];
    char settingsCapture[100];
    char settingsFilename[100];
    char settingsCamIndex[100];
    char settingsSaveVideo[100];

    // Set variables for blob detection
    double blobSizeX = 0;
    double blobSizeY = 0;
    double blobArea = 0;
    double boxArea = 0;
    double ratio = 0;
    int detect=0;
    int j = 0;
    double area=0;
    double blobCircularity=0;
    double pupilDiameter=0;
    unsigned int result=0;
    unsigned int purkinje=0;
    CvBlobs blobs;
    CvBlobs blobs_p;
    CvTracks tracks;

int inputKey=0;


    // Main Loop
    while( 1 )
    {

        //Create trackbars for the settings window
        cvCreateTrackbar("Pupil threshold","Settings",&pupilThreshold,255,PupilThreshold);
        cvCreateTrackbar("Purkinje threshold","Settings",&purkinjeThreshold,255,PurkinjeThreshold);
        cvCreateTrackbar("Min Pupil Area","Settings",&minArea,5000,MinArea);
        cvCreateTrackbar("Max Pupil Area","Settings",&maxArea,20000,MaxArea);
        cvCreateTrackbar("Min Purkinje Area","Settings",&minPurkinjeArea,100,MinPkjArea);
        cvCreateTrackbar("Max Purkinje Area","Settings",&maxPurkinjeArea,500,MaxPkjArea);
        cvCreateTrackbar("Circularity","Settings",&pupilCircularity,100,PupilCircularity);
        cvCreateTrackbar("Record","Settings",&dataRecord,1,DataRecord);
        cvCreateTrackbar("Calibrate","Settings",&homographicCalibration,1,TrackingCalibration);
        cvCreateTrackbar("Track","Settings",&startEyetracker,1,TrackingEye);
        cvCreateTrackbar("Plot","Settings",&plotPupilDiameter,1,PlotGnuplot);
        cvCreateTrackbar("Pixel-mm calibrate","Settings",&mmToPixelCalibration,1,Pixelmm);


        // Gnuplot path settings
        gnuplotPath[0]='\0';
        getcwd(path,255);
        strcat(gnuplotPath,"cd '");
        strcat(gnuplotPath,path);
        strcat(gnuplotPath,"' \n");

        // Enter data
        while( 1 ){
            strcpy(subjectName,DataEnter("Subject's data","Name(mandatory):"));
            if(strlen(subjectName)>0){break;}
        }
        strcpy(subjectAge,DataEnter("Subject's data","Age (optional):"));
        cvDestroyWindow("Subject's data");

        // Plot or capture
         while (1){
            strcpy(settingsPlot,DataEnter("Capture settings","Plot (p) or capture (c)?"));
            if (*settingsPlot == 'p' || *settingsPlot == 'c') {break;}
         }


        if (*settingsPlot == 'p' ) {
            chdir(subjectName);

                FILE *gpp = popen("gnuplot -persist", "w");
                fprintf(gpp, "set datafile missing \"?\" \n");
                fprintf(gpp, "set ylabel 'Diameter [milimeters]'  \n");
                fprintf(gpp, "set xlabel 'Time [seconds]' \n");
                fprintf(gpp, "set yrange [0:10] \n");
                fprintf(gpp, "set grid lt 0 lw 1 lc rgb \"black\" \n");
                fprintf(gpp, "set border linecolor rgb \"black\" \n");
                fprintf(gpp, "set obj 1 rectangle behind from screen 0,0 to screen 1,1 \n");
                fprintf(gpp, "plot \"captureData.dmt\" using 1:($2) title '' with lines lc rgb \"red\"\n");
                fflush(gpp);
                fprintf(gpp, "replot \n");

                FILE *gp1 = popen("gnuplot -persist", "w");
                fprintf(gpp, "set datafile missing \"?\" \n");
                fprintf(gp1, "set ylabel 'x position [pixels]' \n");
                fprintf(gp1, "set xlabel 'Time [seconds]' \n");
                fprintf(gp1, "set yrange [0:1280] \n");
                fprintf(gp1, "set grid lt 0 lw 1 lc rgb \"black\" \n");
                fprintf(gp1, "set border linecolor rgb \"black\" \n");
                fprintf(gp1, "set obj 1 rectangle behind from screen 0,0 to screen 1,1 \n");
                fprintf(gp1, "plot \"captureData.dmt\" using 1:($3) title '' with lines lc rgb \"green\"\n");
                fflush(gp1);
                fprintf(gp1, "replot \n");

                FILE *gp2 = popen("gnuplot -persist", "w");
                fprintf(gpp, "set datafile missing \"?\" \n");
                fprintf(gp2, "set ylabel 'y position [pixels]' \n");
                fprintf(gp2, "set xlabel 'Time [seconds]' \n");
                fprintf(gp2, "set yrange [0:800] \n");
                fprintf(gp2, "set grid lt 0 lw 1 lc rgb \"black\" \n");
                fprintf(gp2, "set border linecolor rgb \"black\" \n");
                fprintf(gp2, "set obj 1 rectangle behind from screen 0,0 to screen 1,1 \n");
                fprintf(gp2, "plot \"captureData.dmt\" using 1:($4) title '' with lines lc rgb \"blue\"\n");
                fflush(gp2);
                fprintf(gp2, "replot \n");

            chdir("..");
        }
        if (*settingsPlot == 'c' ) {

        // Initialize capture
        IplImage *rgb_image;
        CvCapture *captureVideo;


        // Capture settings
        while (1){
            strcpy(settingsCapture,DataEnter("Capture settings","Capture from file (f) or camera (c)?:"));
            if (*settingsCapture == 'f' || *settingsCapture == 'c') {break;}
        }


        if (*settingsCapture == 'f' ) {
            while(1){
                strcpy(settingsFilename,DataEnter("Capture settings","Enter Filename (with extension):"));
                captureVideo = cvCaptureFromFile(settingsFilename);
                if (captureVideo){break;}
            }
        }

        if (*settingsCapture == 'c' ) {
            while(1){
                strcpy(settingsCamIndex,DataEnter("Capture settings","Enter camera index:"));
                captureVideo = cvCaptureFromCAM(settingsCamIndex);
                //Options
                // cvSetCaptureProperty( captureVideo, CV_CAP_PROP_FRAME_WIDTH, 640 );
                // cvSetCaptureProperty( captureVideo, CV_CAP_PROP_FRAME_HEIGHT, 480 );
                // cvSetCaptureProperty( captureVideo, CV_CAP_PROP_FPS, 30 );
                if(captureVideo){break;}
            }
        }


        while (1){
            strcpy(settingsSaveVideo,DataEnter("Capture settings","Save video file? [y/n]"));
            if (*settingsSaveVideo == 'y' || *settingsSaveVideo == 'n') {break;}
        }

        // Check if device is ready
        if( !captureVideo ) return 1;

        // Get time
        time_t now = time(0);

        // convert now to string form
        char* dt = ctime(&now);

        char genericName[100]="";
        char videoFile[100]="";
        char captureDataFile[100]="";
        char positionFile[100]="";
        char dataFile[100]="";
        char calibrationFile[100]="";


        // Generate files for handling data & Gnuplot Pipes
        mkdir(subjectName,0777);
        chmod(subjectName,0777);
        CvMat* c1 = (CvMat*)cvLoad( "Intrinsics.xml" );
        chdir(subjectName);
        cvSave( "Intrinsics.xml", c1 );
        strcat(genericName,subjectName);
        strcat(genericName,"-");
        strcat(genericName,dt);

        strcat(captureDataFile,genericName);
        strcat(captureDataFile,".dmt");

        strcat(positionFile,genericName);
        strcat(positionFile,".pst");

        strcat(dataFile,genericName);
        strcat(dataFile,".dat");

        strcat(calibrationFile,genericName);
        strcat(calibrationFile,".cal");

        strcat(videoFile,genericName);
        strcat(videoFile,".avi");

        FILE *captureData = fopen("captureData.dmt","w");
        FILE *position = fopen("position.pst","w");
        FILE *data = fopen("datafile.dat","w");
        FILE *gp = popen("gnuplot -persist", "w");

        //Write dataFile
        fprintf(data,"==Subject's data=======================\n");
        fprintf(data,"Name:\t %s \n",subjectName);
        fprintf(data,"Age:\t %s \n",subjectAge);
        fprintf(data,"Comments:\t %s\n",subjectComments );


        // Video writing settings
        CvVideoWriter *writer = 0;

        int isColor = 1;
        int framesPerSecond = 30;

        if(*settingsSaveVideo == 'y'){
        writer=cvCreateVideoWriter(videoFile,CV_FOURCC('M','J','P','G'),framesPerSecond,cvSize(640,480),isColor);
        }

        j=0;

        double plotTimeStart = GetMicroSeconds();
        double fpsTimer = GetMicroSeconds();


        int roi_minx=1;
        int roi_maxx=639;
        int roi_miny=1;
        int roi_maxy=479;
        int centerx;
        int centery;
        int centerpkjx;
        int centerpkjy;
        int pkjx;
        int pkjy;

        detect=0;
        int counter3=0;
        double distance=0;
        double distanceAcc=0;
        int readCalibration[100];


        CvMat* map_matrix = cvCreateMat(3,3,CV_64FC1);


        //  Display points. You have to change this points according to your display resolution. xml file in the future.
        double dc2[] = {   0,256,512,768,1024,   0,256,512,768,1024,   0,256,512,768,1024,   0,256,512,768,1024,   0,256,512,768,1024,
                            0,  0,  0,  0,  0,   192,192,192,192,192,   384,384,384,384,384,  576,576,576,576,576,  768,768,768,768,768};

        CvMat c2 = cvMat(2, 25, CV_64FC1, dc2 );

        int point_index=0;

        cvFindHomography(c1,&c2,map_matrix,CV_RANSAC, 2.5, 0);

        int saved_point=0;

        //Gnuplot plotting settings
        fprintf(gp, "set ylabel 'Diameter [milimeters]' \n");
        fprintf(gp, "set xlabel 'Time' \n");
        fprintf(gp, "set yrange [0:10] \n");
        fprintf(gp, "set grid lt 0 lw 1 lc rgb \"black\" \n");
        fprintf(gp, "set border linecolor rgb \"black\" \n");
        fprintf(gp, "set obj 1 rectangle behind from screen 0,0 to screen 1,1 \n");
        fprintf(gp, "set datafile missing \"?\" \n");
        fprintf(gp, "plot \"captureData.dmt\" using 1:($2) title '' with lines lc rgb \"red\"\n");


        while(1)
        {

            // Query frame
            rgb_image = cvQueryFrame( captureVideo );

            IplImage *gray_image = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_8U, 1);
            IplImage *equalized_image_rgb = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_8U, 3);
            IplImage *equalized_image_gray = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_8U, 1);
            IplImage *threshold_image = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_8U, 1);
            IplImage *purkinje_image_threshold = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_8U, 1);
            IplImage *mask_image = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_8U, 1);
            IplImage *label_img_pupil = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_LABEL, 1);
            IplImage *label_img_purkinje = cvCreateImage(cvGetSize(rgb_image), IPL_DEPTH_LABEL, 1);
            IplImage *small_threshold_image = cvCreateImage(cvSize(rgb_image->width/2,rgb_image->height/2),IPL_DEPTH_8U, 1);
            IplImage *small_purkinje_image_threshold = cvCreateImage(cvSize(rgb_image->width/2,rgb_image->height/2),IPL_DEPTH_8U, 1);
            IplImage *calibrate_screen = cvCreateImage(cvSize(1024,768),8,3);

            if (dataRecord==0){
                if (j==1) break;
            }

            if( !cvGetWindowHandle("Settings") ) return 0;
            if( !cvGetWindowHandle("Pupil tracking") ) {
                cvReleaseCapture(&captureVideo);
                break;
            }

            // calculate current FPS
            ++frameCounter;

            if (dataRecord == 0){
                plotTimeStart = GetMicroSeconds();
            }

            if (dataRecord == 1) plotTime = (double)GetMicroSeconds() - plotTimeStart;

            double fps;

            if (frameCounter == 20 ){
                frameCounter = 0;
                fps = 20 / ( GetMicroSeconds() - fpsTimer );
                fpsTimer = GetMicroSeconds();
            }

            // Check for frames
            if( !rgb_image ) break;

            // Convert to grayscale
            cvCvtColor(rgb_image,gray_image,CV_RGB2GRAY);

            // Equalize histogram */
            cvEqualizeHist(gray_image,equalized_image_gray);

            // Smooth image for noise reduction
            cvSmooth(equalized_image_gray,equalized_image_gray,CV_GAUSSIAN,5);

            // Generate rgb from grayscale image for color renders
            cvCvtColor(equalized_image_gray,equalized_image_rgb,CV_GRAY2RGB);

            // Threshold image for pupil detection
            cvThreshold(equalized_image_gray, threshold_image, pupilThreshold, 255, CV_THRESH_BINARY_INV);


            // Threshold image for purkinje detection
            cvThreshold(equalized_image_gray, purkinje_image_threshold, purkinjeThreshold, 255, CV_THRESH_BINARY);


            if (detect == 1){
                cvZero(mask_image);
                cvSetImageROI(mask_image, cvRect(centerx-150,centery-50,
                                                300,200));

                cvAddS(mask_image,cvScalar(1),mask_image);
                cvResetImageROI(mask_image);
                cvMul(purkinje_image_threshold,mask_image,purkinje_image_threshold);
                cvRectangle(purkinje_image_threshold,
                            cvPoint(centerx-150,centery-50),
                            cvPoint(centerx+150,centery+150),
                            cvScalar(255));

            }


            cvResize(threshold_image,small_threshold_image);
            cvResize(purkinje_image_threshold,small_purkinje_image_threshold);
            cvShowImage("Threshold pupil image",small_threshold_image);
            cvMoveWindow("Threshold pupil image",401,641);
            cvShowImage("Threshold purkinje image",small_purkinje_image_threshold);
            cvMoveWindow("Threshold purkinje image",722,641);

            if (homographicCalibration == 1){
                cvNamedWindow("Calibrate screen",CV_WINDOW_NORMAL);
                cvSetWindowProperty("Calibrate screen", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);


                CvScalar point_x = cvGet2D(&c2,0,point_index);
                CvScalar point_y = cvGet2D(&c2,1,point_index);

                double x_=point_x.val[0];
                double y_=point_y.val[0];

                cvAddS(calibrate_screen,cvScalar(120,120,120),calibrate_screen);

                cvCircle(  calibrate_screen,
                            cvPoint(x_,y_),
                            25,
                            cvScalar(255,179,128),
                            -1);
                cvCircle(  calibrate_screen,
                            cvPoint(x_,y_),
                            20,
                            cvScalar(255,255,255),
                            -1);
                cvCircle(  calibrate_screen,
                            cvPoint(x_,y_),
                            15,
                            cvScalar(255,179,128),
                            -1);
                cvCircle(  calibrate_screen,
                            cvPoint(x_,y_),
                            10,
                            cvScalar(255,255,255),
                            -1);
                cvCircle(  calibrate_screen,
                            cvPoint(x_,y_),
                            5,
                            cvScalar(255,179,128),
                            -1);


                if (saved_point==1){
                    cvCircle(  calibrate_screen,
                                cvPoint(x_,y_),
                                5,
                                cvScalar(0,255,255),
                                -1);
                }

                cvShowImage("Calibrate screen",calibrate_screen);

                switch (inputKey) {
                            case 81:
                                if (point_index > 0) point_index=point_index-1;
                                saved_point=0;
                                break;
                            case 82:
                                if (point_index > 4) point_index=point_index-5;
                                saved_point=0;
                                break;
                            case 83:
                                if (point_index < 24) point_index=point_index+1;
                                saved_point=0;
                                break;
                            case 84:
                                if (point_index < 21) point_index=point_index+5;
                                saved_point=0;
                                break;
                            case 32:
                                saved_point=1;
                                cvSet2D( c1,0,point_index, cvScalar(pkjx - centerx + 320 ));
                                cvSet2D( c1,1,point_index, cvScalar(pkjy + centery + 240 ));
                                cvFindHomography(c1,&c2,map_matrix,0, .1, 0);
                                cvSave( "Intrinsics.xml", c1 );
                                break;
                            case 8:
                                cvDestroyWindow("Calibrate screen");
                                cvSetTrackbarPos("Calibrate","Settings",0);
                                saved_point=0;
                                break;
                }


            }


            CvPoint pkj1;
            CvPoint pkj2;

            // Detect pupil blobs
            detect=0;

            result=cvLabel(threshold_image, label_img_pupil, blobs);

            cvFilterByArea(blobs,minArea,maxArea);

            for (CvBlobs::const_iterator it1=blobs.begin(); it1!=blobs.end(); ++it1)
            {
                CvContourPolygon *polygon = cvConvertChainCodesToPolygon(&(*it1).second->contour);
                CvContourPolygon *sPolygon = cvSimplifyPolygon(polygon, 1);
                CvContourPolygon *cPolygon = cvPolygonContourConvexHull(sPolygon);

                blobCircularity=0;
                area = cvContourPolygonArea(cPolygon);
                blobCircularity = cvContourPolygonCircularity(cPolygon);

                if (blobCircularity <  ( 10 - pupilCircularity * .1) ){


                    pupilDiameter=2 * sqrt(area / 3.14159) / mmToPixelRatio;


                    if (mmToPixelCalibration==1){
                        mmToPixelRatio=(2 * sqrt(area / 3.14159))/5;}

                    detect=1;

                    cvRenderContourPolygon(cPolygon, equalized_image_rgb, CV_RGB(255, 0, 0));

                    roi_minx=it1->second->minx;
                    roi_maxx=it1->second->maxx;
                    roi_miny=it1->second->miny;
                    roi_maxy=it1->second->maxy;

                    centerx=it1->second->centroid.x;
                    centery=it1->second->centroid.y;

                }

                delete cPolygon;
                delete sPolygon;
                delete polygon;

            }

            cvReleaseBlobs(blobs);


            sprintf (buf,"FPS: %f Time: %f", fps, plotTime );

            cvPutText(equalized_image_rgb, buf , cvPoint(10, 430), &font, CV_RGB(0,0,0));

            double distance;
            CvTracks tracks;

            // Detect purkinje blobs

            if (detect==1 && startEyetracker==1){
                purkinje=cvLabel(purkinje_image_threshold,label_img_purkinje,blobs_p);
                cvFilterByArea(blobs_p,minPurkinjeArea,maxPurkinjeArea);
                int count=1;
                for (CvBlobs::const_iterator it2=blobs_p.begin(); it2!=blobs_p.end(); ++it2){
                    centerpkjx=it2->second->centroid.x;
                    centerpkjy=it2->second->centroid.y;
                    distance = sqrt((centerpkjx-centerx)*(centerpkjx-centerx)+(centerpkjy-centery)*(centerpkjy-centery));

                    if (distance < 100 ){
                            if (count==1){ pkj1 = cvPoint(it2->second->centroid.x,it2->second->centroid.y);}
                            if (count==2){ pkj2 = cvPoint(it2->second->centroid.x,it2->second->centroid.y);
                            cvLine(equalized_image_rgb, pkj1, pkj2, CV_RGB(0,0,255),1, 8,0);
                            pkjx=(pkj1.x + pkj2.x )/2;
                            pkjy=-(pkj1.y+pkj2.y)/2;
                            }
                            count=count+1;
                    }
                }
                cvReleaseBlobs(blobs_p);
            }

            double dA[] = {pkjx - centerx + 320, pkjy + centery + 240,1};
            CvMat A = cvMat(3, 1, CV_64FC1, dA );
            CvMat* C = cvCreateMat( 3, 1, CV_64FC1);
            cvMatMul( map_matrix,&A, C);
            CvScalar scalar;

            double x_pos;
            double y_pos;
            double w_mat;
            double number;

            scalar = cvGet2D( C, 0, 0);
            x_pos=scalar.val[0];

            scalar = cvGet2D( C, 1, 0);
            y_pos=scalar.val[0];

            scalar = cvGet2D( C, 2, 0);
            w_mat=scalar.val[0];

            switch (detect){
                case 1:
                    switch (dataRecord){
                        case 1:
                            fprintf(captureData,"%g \t %g \t %d \t %d \t %d  \n",plotTime,pupilDiameter,cvRound(x_pos/w_mat),cvRound(y_pos/w_mat),tagFrames);
                            break;
                        case 0:
                            break;
                    }
                    break;
                case 0:
                    cvPutText(equalized_image_rgb, "Blink", cvPoint(10, 50), &font, CV_RGB(255,0,0));
                    switch (dataRecord){
                        case 1:
                            fprintf(captureData,"%g \t %s \t %s \t %s \n",plotTime,"?","?","?");
                             break;
                        case 0:
                            break;
                    }
            }

            switch (inputKey) {
                            case 109:
                                movePointer=1;
                                break;
                            case 110:
                                movePointer=0;
                                break;
                            case 115:
                                tagFrames=1;
                                break;
                            case 101:
                                tagFrames=0;
                                break;
            }

            if (tagFrames==1){cvPutText(equalized_image_rgb, "Tag", cvPoint(10, 100), &font, CV_RGB(60,40,211));}

            if (detect==1  && startEyetracker==1 && homographicCalibration==0 && movePointer==1) MouseMove(x_pos/w_mat,y_pos/w_mat);
            if (detect==1  && startEyetracker==1 && homographicCalibration==1){
                            cvCircle(calibrate_screen,
                            cvPoint(cvRound(x_pos/w_mat),cvRound(y_pos/w_mat)),
                            70,
                            cvScalar(110,110,110),
                            1);
                            cvShowImage("Calibrate screen",calibrate_screen);}

            cvShowImage("Pupil tracking", equalized_image_rgb);
            cvWriteFrame(writer,equalized_image_rgb);

            inputKey = (cvWaitKey( 1 )&0xff);

            if(dataRecord==1 && plotPupilDiameter==1)
            {
                gnuplotFrameCounter++;

                if (gnuplotFrameCounter == 10)
                {
                    fflush(captureData);
                    fflush(position);
                    fflush(gp);
                    fprintf(gp, "replot \n");
                    gnuplotFrameCounter = 0;

                }
            }

            cvReleaseImage(&label_img_pupil);
            cvReleaseImage(&label_img_purkinje);
            cvReleaseImage(&threshold_image);
            cvReleaseImage(&purkinje_image_threshold);
            cvReleaseImage(&mask_image);
            cvReleaseImage(&small_threshold_image);
            cvReleaseImage(&small_purkinje_image_threshold);
            cvReleaseImage(&gray_image);
            cvReleaseImage(&equalized_image_gray);
            cvReleaseImage(&equalized_image_rgb);
            cvReleaseImage(&calibrate_screen);

        }
            cvZero(settings);
            fclose(captureData);
            fclose(position);
            fclose(data);
            pclose(gp);
            dataRecord = 0;

            if( !cvGetWindowHandle("Settings") ) return 0;
            cvShowImage("Settings",settings);
            rename("captureData.dmt",captureDataFile);
            rename("position.pst",positionFile);
            rename("datafile.dat",dataFile);
            chdir("..");

        }
    }
    cvDestroyAllWindows();
}

