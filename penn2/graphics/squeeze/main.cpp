#include <iostream>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <vector>
#include <sstream>
#include <chrono> // timing
#include <time.h>
//#include <boost/random/mersenne_twister.hpp>
//#include <boost/random/uniform_int_distribution.hpp>
#include <zmq.hpp>
#include <boost/circular_buffer.hpp> // to hold recent samples
#include <thread>
#include <stdint.h>
#include <fstream>

#include <signal.h>

#include <unistd.h>
#include <sys/io.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>

#include <random>
#include <algorithm>

//#include "GetPot.h"

using namespace std;
using namespace chrono;
using namespace zmq;
using namespace boost::accumulators;

//string cfgFile("../squeeze.cfg");

int loop = 0;

float emgPower = 0.0;

bool baseline = true;

double baselinePowerMean = 0.0;
double baselinePowerSD = 0.0;

size_t emgState = 0;
size_t prvEmgState = 0;

bool emgClick = false;

// left = 0 / right = 1
int leftRight = 1;

bool quit = false;

bool start = false;

size_t timeStamp = 0;

void powerTh()
{
    size_t dataSize = sizeof(size_t) * 1;
    time_t rawtime;
    time(&rawtime);
    char nameBuffer[24];
    tm * ptm = localtime(&rawtime);
    strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S", ptm);
    string dataFilename = string("click_data_")+string(nameBuffer);
    string dataTdtFilename = string("data_")+string(nameBuffer);

    FILE* pFile;
    pFile = fopen(dataFilename.c_str(), "wb");
    //setvbuf (pFile, NULL, _IOFBF, dataSize);


    FILE* pTdtFile;
    pTdtFile = fopen(dataTdtFilename.c_str(), "wb");
    setvbuf (pTdtFile, NULL, _IOFBF, 10*32*sizeof(float));


    cout<<"thread started"<<endl;
    size_t numChannels = 1;
    vector<double> point(numChannels);

    context_t context(1);

    socket_t subscriber(context, ZMQ_SUB);
    uint64_t hwm = 1;
    //subscriber.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));
    subscriber.connect("ipc:///tmp/signal.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    size_t baselineSamples = 0;

    accumulator_set<float, stats<tag::variance> > acc;

    boost::circular_buffer<double> livePwrSamples;
    livePwrSamples.set_capacity(200);
    for (size_t i=0; i<livePwrSamples.capacity(); i++)
        livePwrSamples.push_back(0);
    double liveAvgPow = 0.0;

    unsigned const numberOfChannels = 64;

    bool prevBaseline = true;

    float buffer[64];
    for (;!quit;) {

        zmq::message_t sig_msg;
        subscriber.recv(&sig_msg);
        memcpy(&timeStamp, sig_msg.data(), sizeof(size_t));

        memcpy(buffer, (size_t*)sig_msg.data()+1, sig_msg.size()-sizeof(size_t));
        point[0] = buffer[0];

        if (!start) {
            continue;
        }

        fwrite(&(buffer[0]), sizeof(float), numberOfChannels, pTdtFile);

        loop += 1;
        if (loop > 5) {
            //livePwrSamples.push_back(point[0]);

            if (baseline) {
                baselineSamples+=1;
                acc(point[0]);
            } else {
                if (prevBaseline) {
                    prevBaseline = false;
                    cout<<" *********************** "<<endl;
                    cout<<" *********************** "<<endl;
                    cout<<"mean "<<mean(acc)<<endl;
                    cout<<"sd "<<sqrt(variance(acc))<<endl;
                    cout<<" *********************** "<<endl;
                    cout<<" *********************** "<<endl;
                    //liveAvgPow = mean(acc);
                    baselinePowerMean = mean(acc);
                    baselinePowerSD = sqrt(variance(acc));
                    string thresholdFilename = string("force_sensor_threshold_")+string(nameBuffer)+string(".txt");
                    ofstream sensorFile(thresholdFilename.c_str());
                    sensorFile<<"baseline mean = "<<baselinePowerMean<<endl;
                    sensorFile<<"baseline standard deviation = "<<baselinePowerSD<<endl;
                    sensorFile<<"if (sensor value > 'baseline mean' + 'baseline standard deviation' * 8) a click is registered"<<endl;
                    sensorFile<<"'baseline mean' + 'baseline standard deviation' * 8 = "<<(baselinePowerMean + baselinePowerSD*8.0)<<endl;
                    sensorFile.close();
                }

                baselinePowerMean = 0.6;
                cout<<"point[0] "<<point[0]<<endl;
                cout<<"threshold: "<<(baselinePowerMean + baselinePowerSD*8.0)<<endl;
                //cout<<"emgState: "<<emgState<<endl;
                cout<<"emgClick: "<<int(emgClick)<<endl;
                cout<<"timsestamp: "<<timeStamp<<endl;
                //cout<<"prvEmgState "<<prvEmgState<<endl;

                if (point[0] > (baselinePowerMean)){// + baselinePowerSD*8.0)) {
                    emgClick = true;
                } else {
                    emgClick = false;
                }
                /*    emgState = 1;

                    if ((prvEmgState == 0)&&(!emgClick)){


                        fwrite(&timeStamp, sizeof(size_t),1 , pFile);
                        fwrite(&leftRight, sizeof(int),1 , pFile);
                        cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
                        cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
                        cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
                        cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
                        cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
                        prvEmgState = 1;
                    }
                } else  {
                    if (emgState==1) {
                        emgClick = false;
                    }
                    emgState = 0;
                    prvEmgState = 0;
                }*/
            }

            loop = 0;
        }

    } // for

    cout<<"quit thread"<<endl;

  fclose(pFile);
  fclose(pTdtFile);
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

void signal_callback_handler(int signum) {
    cout<<"int signal"<<endl;
    signal(signum, SIG_IGN);
    quit = true;
}

int main(int argc, char** argv)
{
    time_t rawtime;
    time(&rawtime);
    char nameBuffer[24];
    tm * ptm = localtime(&rawtime);
    strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S", ptm);
    string imageFilename = string("image_data_")+string(nameBuffer);
    string pauseFilename = string("pause_data_")+string(nameBuffer);

    FILE* pFileImage;
    pFileImage = fopen(imageFilename.c_str(), "wb");

    FILE* pFilePause;
    pFilePause = fopen(pauseFilename.c_str(), "wb");

    signal(SIGINT, signal_callback_handler);

    //GetPot cl(argc, argv);

    // check if config exists
    /*ifstream testCfgFile(cfgFile.c_str());
    if (! testCfgFile.good()) {
        cout<<"Could not open the config file: "<<cfgFile<<endl;
        return 1;
    } else {
        testCfgFile.close();
    }*/

    cout<<"test"<<endl;

    // parse config file
    /*GetPot ifile(cfgFile.c_str(), "#", "\n");
    ifile.print();

    size_t numImages = ifile("numberOfCards", 10);
    cout<<"numImages: "<<numImages<<endl;

    size_t useSmallCards = ifile("useSmallCards", 1);
    cout<<"useSmallCards: "<<useSmallCards<<endl;

    size_t playSoundEffect = ifile("playSoundEffect", 1);
    cout<<"playSoundEffect: "<<playSoundEffect<<endl;

    size_t playTheme = ifile("playTheme", 1);
    cout<<"playTheme: "<<playTheme<<endl;

    size_t dCorrectSwitch = ifile("displayCorrectIncorrect", 0);
    cout<<"dCorrectSwitch: "<<dCorrectSwitch<<endl;

    size_t dScoreSwitch = ifile("displayScore", 1);
    cout<<"dScoreSwitch: "<<dScoreSwitch<<endl;*/

    boost::circular_buffer<double> pwrCBuff1;
    pwrCBuff1.set_capacity(50);
    boost::circular_buffer<double> pwrCBuff2;
    pwrCBuff2.set_capacity(20);
    cout<<"test"<<endl;


    srand (time(NULL));

    TTF_Font *font = NULL;
    TTF_Font *fontMed = NULL;
    TTF_Font *fontBig = NULL;
    TTF_Font *fontBigBack = NULL;

    size_t sw = 1024;
    size_t sh = 768;

    const unsigned numImages = 4;
    SDL_Surface *screen;	//This pointer will reference the backbuffer
    vector<SDL_Surface *> image(numImages);	//This pointer will reference our bitmap sprite
    vector<SDL_Surface *> temp(numImages);	//This pointer will temporarily reference our bitmap sprite
    SDL_Rect src, dest, wholeScreen;	//These rectangles will describe the source and destination regions of our blit
    SDL_Surface *background;
    SDL_Surface *tmpBackground;

    //We must first initialize the SDL video component, and check for success
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    TTF_Init();

    cout<<"test"<<endl;

    font = TTF_OpenFont("../Arial.ttf", 28);
    fontMed = TTF_OpenFont("../Arial_Bold.ttf", 38);
    fontBig = TTF_OpenFont("../Arial_Bold.ttf", 92);
    fontBigBack = TTF_OpenFont("../Arial_Bold.ttf", 95);
    SDL_Color textColor = { 255, 255, 255 };
    SDL_Color textColorBlue = { 0, 0, 110 };

    /*int flags=MIX_INIT_MP3;
    int initted=Mix_Init(flags);
    if(initted&flags != flags) {
      printf("Mix_Init: Failed to init required ogg and mod support!\n");
      printf("Mix_Init: %s\n", Mix_GetError());
    }
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 1024)==-1) {
      printf("Mix_OpenAudio: %s\n", Mix_GetError());
      exit(2);
    }*/

    //SDL_ShowCursor(0);

    //When this program exits, SDL_Quit must be called
    atexit(SDL_Quit);

    //Set the video mode with 16bit colour and double-buffering
    SDL_putenv("SDL_VIDEO_WINDOW_POS=0,0");
    screen = SDL_SetVideoMode(sw, sh, 16, SDL_DOUBLEBUF /*| SDL_FULLSCREEN*/);
    if (screen == NULL) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    //Load the bitmap into a temporary surface, and check for success
    /*for (size_t i=0; i<numImages; i++) {
        ostringstream ss;
        if (useSmallCards)
            ss<<"./small/";
        ss<<(i+1)<<".bmp";
        cout<<ss.str()<<endl;
        temp[i] = SDL_LoadBMP(ss.str().c_str());
        if (temp[i] == NULL) {
            printf("Unable to load bitmap: %s\n", SDL_GetError());
            return 1;
        }
    }
    tmpBackground = SDL_LoadBMP("b2.bmp");
    */
    temp[0] = SDL_LoadBMP("../circle.bmp");
    temp[1] = SDL_LoadBMP("../square.bmp");
    temp[2] = SDL_LoadBMP("../triangle.bmp");
    temp[3] = SDL_LoadBMP("../circle2.bmp");

    //Release the temporary surface
    /*for (size_t i=0; i<numImages; i++) {
        image[i] = SDL_DisplayFormat(temp[i]);
        SDL_FreeSurface(temp[i]);
    }
    background = SDL_DisplayFormat(tmpBackground);
    SDL_FreeSurface(tmpBackground);*/
    image[0] = SDL_DisplayFormat(temp[0]);
    image[1] = SDL_DisplayFormat(temp[1]);
    image[2] = SDL_DisplayFormat(temp[2]);
    image[3] = SDL_DisplayFormat(temp[3]);
    SDL_FreeSurface(temp[0]);
    SDL_FreeSurface(temp[1]);
    SDL_FreeSurface(temp[2]);
    SDL_FreeSurface(temp[3]);

    //Construct the source rectangle for our blit
    src.x = 0;
    src.y = 0;
    src.w = image[0]->w;	//Use image->w to display the entire width of the image
    src.h = image[0]->h;	//Use image->h to display the entire height of the image

    //Construct the destination rectangle for our blit
    dest.x = sw/2-image[0]->w/2;		//Display the image at the (X,Y) coordinates (100,100)
    dest.y = sh/2-image[0]->h/2;
    dest.w = image[0]->w;	//Ensure the destination is large enough for the image's entire width/height
    dest.h = image[0]->h;

    wholeScreen.x = 0;
    wholeScreen.y = 0;
    wholeScreen.w = sw;
    wholeScreen.h = sh;

    std::thread helper1(powerTh);
    // countdown for baseline emg
    size_t countdown = 10;
    for (size_t t=0; t<countdown; t++) {
        SDL_FillRect(screen, &wholeScreen, SDL_MapRGB(screen->format, 255, 255, 255));

        ostringstream scoreSS;
        scoreSS<<countdown-t;
        SDL_Surface *message = TTF_RenderText_Blended(fontBig, scoreSS.str().c_str(), textColorBlue);
        apply_surface(sw/2-message->w/2, 350, message, screen);
        SDL_FreeSurface(message);

        if (argc > 1) {
            SDL_Surface *message = TTF_RenderText_Blended(fontBig, "Relax", textColorBlue);
            apply_surface(sw/2-message->w/2, 200, message, screen);
            SDL_FreeSurface(message);
        }

        SDL_Flip(screen);

        usleep(1000000);

        if (t > 2) {
            start = true;
        } if (t > 7) {
            baseline = false;
        }
    }

    if (argc > 1) {
        quit = true;
        usleep(1500000);

        helper1.join();

        fclose(pFilePause);
        fclose(pFileImage);

        cout<<"q1"<<endl;
        //Release the surface
        for (size_t i=0; i<numImages; i++)
            SDL_FreeSurface(image[i]);

        //SDL_FreeSurface(background);

        TTF_CloseFont(font);
        TTF_CloseFont(fontMed);
        TTF_CloseFont(fontBig);
        TTF_CloseFont(fontBigBack);
        TTF_Quit();

      return 0;

    }

    usleep(1500000);

    // background image
    //apply_surface(0, 0, background, screen);

    size_t firstImage = 0;
    fwrite(&timeStamp, sizeof(size_t),1 , pFileImage);
    fwrite(&firstImage, sizeof(size_t),1 , pFileImage);
    //Blit the first image to the backbuffer
    SDL_FillRect(screen, &wholeScreen, SDL_MapRGB(screen->format, 255, 255, 255));
    SDL_BlitSurface(image[0], &src, screen, &dest);

    // show score
    /*size_t score = 100;
    ostringstream scoreSS;
    scoreSS<<"Score: "<<score;
    SDL_Surface *message = TTF_RenderText_Blended(fontMed, scoreSS.str().c_str(), textColorBlue);
    apply_surface(sw/2-image[0]->w/2 - 50, 150, message, screen);
    SDL_FreeSurface(message);*/

    /*ostringstream handSS;
    handSS<<"Left Hand";
    message = TTF_RenderText_Blended(font, handSS.str().c_str(), textColorBlue);
    apply_surface(sw/2-image[0]->w/2 - 50, 550, message, screen);
    SDL_FreeSurface(message);*/

    //Flip the backbuffer to the primary
    SDL_Flip(screen);

    auto startTrial = high_resolution_clock::now();
    auto start = high_resolution_clock::now();
    auto end = high_resolution_clock::now();

    SDL_Event event;
    //bool clicked = false;

    //size_t prevImg = 0;
    //size_t prevImg2 = -1; // No image to compare against

    //int state = 0; // 0: showing image
                 // 1: black screen between images

    //boost::random::mt19937 gen;
    //boost::random::uniform_int_distribution<> dist(0, numImages-1);
    //boost::random::uniform_int_distribution<> tdist(0, 1);

    //bool powerReady = false;

    //bool pause = false;
    //bool correct = false;

    //bool midSession = false;

    int sState = 0;
    size_t imgNum = 0;

    int squeezeMs = 500;
    int restMs = 1500;

    int sRMs = 1500;

    int numSqueezes = 0;

    default_random_engine rng(random_device{}());
    uniform_int_distribution<int> dist(0,500);

    while( quit == false ) {
      // background image
      //apply_surface(0, 0, background, screen);
      SDL_FillRect(screen, &wholeScreen, SDL_MapRGB(screen->format, 255, 255, 255));

      // show score
      /*ostringstream scoreSS;
      scoreSS<<"Score: "<<score;
      if (dScoreSwitch) {
        SDL_Surface *message = TTF_RenderText_Blended(fontMed, scoreSS.str().c_str(), textColorBlue);
        apply_surface(sw/2-image[0]->w/2 - 50, 150, message, screen);
        SDL_FreeSurface(message);
      }

      ostringstream handSS;
      if (leftRight == 0)
          handSS<<"Left Hand";
      else
          handSS<<"Right Hand";

      message = TTF_RenderText_Blended(font, handSS.str().c_str(), textColorBlue);
      apply_surface(sw/2-image[0]->w/2 - 50, 550, message, screen);
      SDL_FreeSurface(message);
      */

      if( SDL_PollEvent( &event ) )
      {

          /*if( event.type == SDL_MOUSEBUTTONDOWN ) {
            clicked = true;
            printf("Mouse Button 1(left) is pressed.\n");
          }*/

          /*if( event.type == SDL_KEYDOWN ) {
              switch( event.key.keysym.sym ){
                case SDLK_SPACE:
                  pause = !pause;
                  fwrite(&timeStamp, sizeof(size_t),1 , pFilePause);
                  if ((!pause) && midSession) {
                    startTrial = high_resolution_clock::now();
                    midSession = false;
                  }
                  break;
                case SDLK_ESCAPE:
                  quit = true;
                  break;
                case 'r':
                   leftRight = 1;
                   break;
                case 'l':
                   leftRight = 0;
                   break;
                default:
                  break;
                }
          }*/

          //If the user has Xed out the window
          if( event.type == SDL_QUIT )
          {
              cout<<"quit recv"<<endl;
              //Quit the program
              quit = true;
              break;
          }
      }

      //clicked |= emgClick;

      end = high_resolution_clock::now();
      milliseconds ms = duration_cast<milliseconds>(end - start);
      milliseconds msTrial = duration_cast<milliseconds>(end - startTrial);

      // 1 sec squeeze
      // 2 secs rest
      // start with rest

      if (sState == 0) {
          if (ms.count() > restMs) {
              start = high_resolution_clock::now();
              sState = 1;
          } else if (emgClick) {
              start = high_resolution_clock::now();
          }
      }
      else if (sState == 1) {
          if (ms.count() > squeezeMs) {
              start = high_resolution_clock::now();
              sState = 0;
              numSqueezes += 1;
              restMs = sRMs + dist(rng);
          }
      }

      if (sState == 0) {
          if (!emgClick) {
            SDL_BlitSurface(image[0], &src, screen, &dest);
          } else {
              SDL_BlitSurface(image[3], &src, screen, &dest);
          }
          imgNum = 0;
      } else if (sState == 1) {
          if (emgClick) {
              SDL_BlitSurface(image[2], &src, screen, &dest);
              imgNum = 2;
          } else {
              SDL_BlitSurface(image[1], &src, screen, &dest);
              imgNum = 1;
          }
      }

      ostringstream numss;
      numss<<numSqueezes;
      SDL_Surface *message = TTF_RenderText_Blended(fontMed, numss.str().c_str(), textColorBlue);
      apply_surface(20, 20, message, screen);
      SDL_FreeSurface(message);


      SDL_Flip(screen);

      fwrite(&timeStamp, sizeof(size_t),1 , pFileImage);
      fwrite(&imgNum, sizeof(size_t),1 , pFileImage);
    }
    cout<<"quit main"<<endl;

    helper1.join();

    fclose(pFilePause);
    fclose(pFileImage);

    cout<<"q1"<<endl;
    //Release the surface
    for (size_t i=0; i<numImages; i++)
        SDL_FreeSurface(image[i]);

    //SDL_FreeSurface(background);

    TTF_CloseFont(font);
    TTF_CloseFont(fontMed);
    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontBigBack);
    TTF_Quit();

  return 0;
}

