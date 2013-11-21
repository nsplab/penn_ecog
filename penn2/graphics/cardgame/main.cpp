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
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <zmq.hpp>
#include <boost/circular_buffer.hpp> // to hold recent samples
#include <thread>
#include <stdint.h>

#include <unistd.h>
#include <sys/io.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>

#include "GetPot.h"

using namespace std;
using namespace chrono;
using namespace zmq;
using namespace boost::accumulators;

string cfgFile("../cardgame.cfg");

int loop = 0;

float emgPower = 0.0;

bool baseline = true;

double baselinePowerMean = 0.0;
double baselinePowerSD = 0.0;

size_t emgState = 0;
size_t prvEmgState = 0;

bool emgClick = false;
  
// left = 0 / right = 1
int leftRight = 0;

bool quit = false;

size_t timeStamp = 0;

void powerTh()
{

  size_t dataSize = sizeof(size_t) * 1;
    time_t rawtime;
    time(&rawtime);
    char nameBuffer[24];
    tm * ptm = localtime(&rawtime);
    strftime(nameBuffer, 24, "%a_%d.%m.%Y_%H:%M:%S", ptm);
    string dataFilename = string("data_click_")+string(nameBuffer);

  FILE* pFile;
  pFile = fopen(dataFilename.c_str(), "wb");
  //setvbuf (pFile, NULL, _IOFBF, dataSize);

  cout<<"thread started"<<endl;
  size_t numChannels = 1;
  vector<double> point(numChannels);

  context_t context(1);

  socket_t subscriber(context, ZMQ_SUB);
  uint64_t hwm = 1; 
  //subscriber.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));
  subscriber.connect("ipc:///tmp/sig.pipe");
  subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

  size_t baselineSamples = 0;

  accumulator_set<float, stats<tag::variance> > acc;

  boost::circular_buffer<double> livePwrSamples;
  livePwrSamples.set_capacity(200);
  for (size_t i=0; i<livePwrSamples.capacity(); i++)
    livePwrSamples.push_back(0);
  double liveAvgPow = 0.0;

//  auto start = high_resolution_clock::now();
//  auto end = high_resolution_clock::now();
//size_t timeStamp = 0;
  float buffer[64];
  for (;!quit;)   {


//  cout<<"recv"<<endl;
  zmq::message_t sig_msg;
  subscriber.recv(&sig_msg);
  memcpy(&timeStamp, sig_msg.data(), sizeof(size_t));

//cout<<"r: "<<(*(char *)sig_msg.data())<<endl;
//cout<<"recvd"<<endl;
//  cout<<"sz "<<sig_msg.size()<<endl;
//  memcpy(&timeStamp, sig_msg.data(), sizeof(size_t));
//  if (timeStamp % 1000 == 0)
cout<<"timestamp: "<<timeStamp<<endl;

  memcpy(buffer, (size_t*)sig_msg.data()+1, sig_msg.size()-sizeof(size_t));
  //cout<<"done"<<endl;

//  for (size_t t=0; t<(sig_msg.size()/sizeof(float)); t++) {
    //cout<<"b:"<<buffer[t]<<" ";

// force sensor
    point[0] = buffer[0];

//cout<<"p0: "<<buffer[0]<<endl;
    //fwrite(&(point[0]), 1,sizeof(float) , pFile);
//    fft.AddPoints(point);
//  }
  //cout<<endl;

  loop += 1;
  if (loop > 5) {
//  cout<<"timestamp: "<<timeStamp<<endl;
//      cout<<"l: "<<loop<<endl;
/*    if (fft.Process()) {
      fft.GetPower(powers);
      double lowerPowers = 0.0;
      for (size_t tt=0; tt<10; tt++) {
          lowerPowers += powers[0][tt];
        }
*/
//      cout<<"lowerPower: "<<lowerPowers<<endl;
//	cout<<"force: "<<point[0]<<endl;

//      fwrite(&lowerPowers, sizeof(double), 1 , pFile);

      //livePwrSamples.push_back(powers[0][8]);
      livePwrSamples.push_back(point[0]);


      if (baseline) {
	cout<<"baseline 2"<<endl;
        baselineSamples+=1;
        //acc(powers[0][8]);
cout<<"point[0]"<<point[0]<<endl;
        acc(point[0]);
//        cout<<"baselineSamples "<<baselineSamples<<endl;
//        end = high_resolution_clock::now();
//        milliseconds ms = duration_cast<milliseconds>(end - start);
//        cout<<"ms "<<ms.count()<<endl;

      if (baselineSamples > 1000){
        baseline = false;
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
	
//	usleep(10000000);
      }
        } else {
          //liveAvgPow += powers[0][8]/float(livePwrSamples.capacity());
cout<<"point[0] "<<point[0]<<endl;
//          liveAvgPow += point[0]/float(livePwrSamples.capacity());
//cout<<"liveAvgPow "<<liveAvgPow<<endl;
//          liveAvgPow -= livePwrSamples[0]/float(livePwrSamples.capacity());
//cout<<"liveAvgPow "<<liveAvgPow<<endl;
cout<<"threshold: "<<(baselinePowerMean + baselinePowerSD*8.0)<<endl;
cout<<"emgState: "<<emgState<<endl;
cout<<"emgClick: "<<int(emgClick)<<endl;
cout<<"timsestamp: "<<timeStamp<<endl;
cout<<"prvEmgState "<<prvEmgState<<endl;


          if (point[0] > (baselinePowerMean + baselinePowerSD*8.0)) {
              emgState = 1;
          if (emgState==0) {
              if ((prvEmgState == 0)&&!emgClick){
                emgClick = true;
          	fwrite(&timeStamp, sizeof(size_t),1 , pFile);
          	fwrite(&leftRight, sizeof(int),1 , pFile);
            cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
            cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
            cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
            cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
            cout<<"* EMG clicked * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< "<<endl;
            prvEmgState = 1;
                }
            } } else  { //if (emgState==1)
                emgClick = false;
              emgState = 0;
              prvEmgState = 0;
            }

        }


      //powerReady = true;
      //pwrCBuff1.push_back(powers[0][2]);
      //pwrCBuff2.push_back(powers[0][2]);
      //cout<<"power: "<<powers[0][7]<<" "<<powers[0][8]<<" "<<powers[0][5]<<" "<<powers[0][6]<<" "<<endl;
      //if (powers[0][2] > 0.0004) {
        //clicked = true;

        //SDL_Surface *message = TTF_RenderText_Solid(font, "Clicked", textColor);
        //apply_surface(sw/2+image[0]->w/2, 150, message, screen);
        //}
      //}
    loop = 0;
  }

  } // for

  fclose(pFile);
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

int main(int argc, char** argv)
{
  /*TCLAP::CmdLine cmd("n-back game", ' ', "0.1");
  TCLAP::ValueArg<int> numImagesArg("n","nimages","number of images", true, 5, "int", cmd);
  TCLAP::SwitchArg smallSwitch("s","small"," small cards", cmd, false);
  TCLAP::SwitchArg dCorrectSwitch("d","dcorrect"," do not show correct/incorrect", cmd, false);

  cmd.parse(argc, argv);*/

  //cout<<"number of images: "<<numImagesArg.getValue()<<endl;
  //cout<<"small cards: "<<smallSwitch.getValue()<<endl;

  GetPot cl(argc, argv);

  // check if config exists
  ifstream testCfgFile(cfgFile.c_str());
  if (! testCfgFile.good()) {
      cout<<"Could not open the config file: "<<cfgFile<<endl;
      return 1;
  } else {
      testCfgFile.close();
  }

cout<<"test"<<endl;
  // parse config file
  GetPot ifile(cfgFile.c_str(), "#", "\n");
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

  //size_t numImages= numImagesArg.getValue();
  //size_t sw = 1680;
  //size_t sh = 1050;
  size_t sw = 1000;
  size_t sh = 700;

  SDL_Surface *screen;	//This pointer will reference the backbuffer
  vector<SDL_Surface *> image(numImages);	//This pointer will reference our bitmap sprite
  vector<SDL_Surface *> temp(numImages);	//This pointer will temporarily reference our bitmap sprite
  SDL_Rect src, dest;	//These rectangles will describe the source and destination regions of our blit
  SDL_Surface *background;
  SDL_Surface *tmpBackground;

  //We must first initialize the SDL video component, and check for success
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
          printf("Unable to initialize SDL: %s\n", SDL_GetError());
          return 1;
  }

  TTF_Init();

cout<<"test"<<endl;

  font = TTF_OpenFont("Arial.ttf", 28);
  fontMed = TTF_OpenFont("Arial_Bold.ttf", 38);
  fontBig = TTF_OpenFont("Arial_Bold.ttf", 92);
  fontBigBack = TTF_OpenFont("Arial_Bold.ttf", 95);
  SDL_Color textColor = { 255, 255, 255 };
  SDL_Color textColorBlue = { 0, 0, 110 };

cout<<"test"<<endl;

  int flags=MIX_INIT_MP3;
  int initted=Mix_Init(flags);
  if(initted&flags != flags) {
      printf("Mix_Init: Failed to init required ogg and mod support!\n");
      printf("Mix_Init: %s\n", Mix_GetError());
  }
  if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 1024)==-1) {
      printf("Mix_OpenAudio: %s\n", Mix_GetError());
      exit(2);
  }

  Mix_Music *music;
  music=Mix_LoadMUS("theme.mp3");

  if (playTheme)
    Mix_PlayMusic(music, -1);

  Mix_VolumeMusic(100);

  Mix_Chunk * correctChunk = Mix_LoadWAV("correct.wav");
  if(!correctChunk){
      cout<<"Could not load file from disk!"<<endl;
  }
  Mix_Chunk * incorrectChunk = Mix_LoadWAV("incorrect.wav");
  if(!incorrectChunk){
      cout<<"Could not load file from disk!"<<endl;
  }

  Mix_VolumeChunk(correctChunk, MIX_MAX_VOLUME);
  Mix_VolumeChunk(incorrectChunk, MIX_MAX_VOLUME);


  SDL_ShowCursor(0);

  //When this program exits, SDL_Quit must be called
  atexit(SDL_Quit);

  //Set the video mode to fullscreen 640x480 with 16bit colour and double-buffering
  screen = SDL_SetVideoMode(sw, sh, 16, SDL_DOUBLEBUF /*| SDL_FULLSCREEN*/);
  if (screen == NULL) {
          printf("Unable to set video mode: %s\n", SDL_GetError());
          return 1;
  }

  //Load the bitmap into a temporary surface, and check for success
  for (size_t i=0; i<numImages; i++) {
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

  //Release the temporary surface
  for (size_t i=0; i<numImages; i++) {
    image[i] = SDL_DisplayFormat(temp[i]);
    SDL_FreeSurface(temp[i]);
  }
  background = SDL_DisplayFormat(tmpBackground);
  SDL_FreeSurface(tmpBackground);

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


  std::thread helper1(powerTh);
  // countdown for baseline emg
  size_t countdown = 10;
  for (size_t t=0; t<countdown; t++) {
      apply_surface(0, 0, background, screen);

      ostringstream scoreSS;
      scoreSS<<countdown-t;
      SDL_Surface *message = TTF_RenderText_Blended(fontBig, scoreSS.str().c_str(), textColorBlue);
      apply_surface(sw/2-message->w/2, 350, message, screen);
      SDL_FreeSurface(message);

      SDL_Flip(screen);

      usleep(1000000);

    }

  usleep(1000000);

  // background image
  apply_surface(0, 0, background, screen);

  //Blit the first image to the backbuffer
  SDL_BlitSurface(image[0], &src, screen, &dest);

  // show score
  size_t score = 100;
  ostringstream scoreSS;
  scoreSS<<"Score: "<<score;
  SDL_Surface *message = TTF_RenderText_Blended(fontMed, scoreSS.str().c_str(), textColorBlue);
  apply_surface(sw/2-image[0]->w/2 - 50, 150, message, screen);
  SDL_FreeSurface(message);
  
  ostringstream handSS;
  handSS<<"Left Hand";
  message = TTF_RenderText_Blended(font, handSS.str().c_str(), textColorBlue);
  apply_surface(sw/2-image[0]->w/2 - 50, 550, message, screen);
  SDL_FreeSurface(message);

  //Flip the backbuffer to the primary
  SDL_Flip(screen);

  auto startTrial = high_resolution_clock::now();
  auto start = high_resolution_clock::now();
  auto end = high_resolution_clock::now();

  SDL_Event event;
  bool clicked = false;

  size_t prevImg = 0;
  size_t prevImg2 = -1; // No image to compare against

  int state = 0; // 0: showing image
                 // 1: black screen between images

  boost::random::mt19937 gen;
  boost::random::uniform_int_distribution<> dist(0, numImages-1);
  boost::random::uniform_int_distribution<> tdist(0, 1);

  //size_t dataSize = sizeof(float) * 22;
  //FILE* pFile;
  //pFile = fopen("data", "wb");
  //setvbuf (pFile, NULL, _IOFBF, dataSize*sizeof(float));

  bool powerReady = false;

  bool pause = false;
  bool correct = false;

  bool midSession = false;


  while( quit == false ) {
  //cout<<"recv"<<endl;
//zmq::message_t sig_msg;
  //subscriber.recv(&sig_msg);
//  subscriber.recv(&sig_msg);
//cout<<"r: "<<(*(char *)sig_msg.data())<<endl;
//  cout<<"recvd"<<endl;
//  cout<<"sz "<<sig_msg.size()<<endl;
//  memcpy(&timeStamp, sig_msg.data(), sizeof(size_t));
//cout<<"timestamp: "<<timeStamp<<endl;
      // background image
      apply_surface(0, 0, background, screen);
      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 97, 97, 97));

      // show score
      ostringstream scoreSS;
      scoreSS<<"Score: "<<score;
      SDL_Surface *message = TTF_RenderText_Blended(fontMed, scoreSS.str().c_str(), textColorBlue);
      apply_surface(sw/2-image[0]->w/2 - 50, 150, message, screen);
      SDL_FreeSurface(message);

      ostringstream handSS;
      if (leftRight == 0)
	      handSS<<"Left Hand";
      else 
	      handSS<<"Right Hand";

      message = TTF_RenderText_Blended(font, handSS.str().c_str(), textColorBlue);
      apply_surface(sw/2-image[0]->w/2 - 50, 550, message, screen);
      SDL_FreeSurface(message);


      if( SDL_PollEvent( &event ) )
      {

          if( event.type == SDL_MOUSEBUTTONDOWN ) {
            clicked = true;
            printf("Mouse Button 1(left) is pressed.\n");


          }

          if( event.type == SDL_KEYDOWN ) {
              switch( event.key.keysym.sym ){
                case SDLK_SPACE:
                  pause = !pause;
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
          }

          //If the user has Xed out the window
          if( event.type == SDL_QUIT )
          {
              //Quit the program
              quit = true;
          }
      }

      clicked |= emgClick;

      end = high_resolution_clock::now();
      milliseconds ms = duration_cast<milliseconds>(end - start);
      milliseconds msTrial = duration_cast<milliseconds>(end - startTrial);
      if (msTrial.count() > 90000) {
        pause = true;
        midSession = true;

        apply_surface(0, 0, background, screen);

        ostringstream scoreSS;
        scoreSS<<"Your current score: "<<score;
        SDL_Surface *message = TTF_RenderText_Blended(fontBig, scoreSS.str().c_str(), textColorBlue);
        apply_surface(sw/2-message->w/2, 350, message, screen);
        SDL_FreeSurface(message);
      }

      if (!pause) {
        if (state == 0) {
            if (ms.count() >= 2000) {
                cout<<"Elapsed nanosecs: "<<ms.count()<<endl;

                if (prevImg2 != -1) { // Score not changed for first image
                    if ((prevImg == prevImg2) && clicked) {
                      cout << "Match Identified Correctly\n";
                      correct = true;
                      score += 1;
                      if (playSoundEffect) {
                      	Mix_PlayChannel(-1, correctChunk, 0);
			cout<<"play effect sound"<<endl;
			}
	            }
                    else if ((prevImg != prevImg2) && !clicked) {
                      cout << "Non-match Identified Correctly\n";
                      correct = true;
                      score += 1;
                    }
                    else {
                        correct = false;
                      cout << "Incorrect\n";
                      score -= 1;
                      if (playSoundEffect)
                      if ((prevImg != prevImg2) && clicked) {
		 	  cout<<"play effect sound"<<endl;
                          Mix_PlayChannel(-1, incorrectChunk, 0);
			}
                    }
                    cout << prevImg << "\n" << prevImg2 << "\n" << clicked << "\n\n";
                }

                state = 1;
                start = high_resolution_clock::now();
            }
        }
        else if (state == 1) {
            if (ms.count() < 1000) {
                // show a black screen between two images
                //SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

                if (dCorrectSwitch) {
                if (correct) {
                    SDL_Surface *message = TTF_RenderText_Blended(font, "Correct!", textColorBlue);
                    apply_surface(sw/2+image[0]->w/2, 150, message, screen);
                    SDL_FreeSurface(message);
                  } else if (prevImg2!=-1) {
                    SDL_Surface *message = TTF_RenderText_Blended(font, "Incorrect!", textColorBlue);
                    apply_surface(sw/2+image[0]->w/2, 150, message, screen);
                    SDL_FreeSurface(message);

                  }
                  }

                SDL_Flip(screen);
            }
            else {
                // keep same image or change it to a new one
                cout<<"rnd"<<endl;
                size_t imgNum = 0;
                if (tdist(gen) == 1)
                  imgNum = prevImg;
                else
                  imgNum = dist(gen);
                cout<<"done"<<endl;

                cout<<imgNum<<endl;
                // first black to refresh the page
                //SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
                SDL_BlitSurface(image[imgNum], &src, screen, &dest);

                SDL_Flip(screen);

                prevImg2 = prevImg;
                prevImg = imgNum;
                start = high_resolution_clock::now();
                clicked = false;
                emgClick = false;


                state = 0;
            }
        }
      } else {

          SDL_Surface *message = TTF_RenderText_Blended(fontBigBack, "PAUSED", textColorBlue);
          apply_surface(sw/2-message->w/2, 100, message, screen);
          message = TTF_RenderText_Blended(fontBig, "PAUSED", textColor);
          apply_surface(sw/2-message->w/2, 100, message, screen);
          SDL_Flip(screen);
          SDL_FreeSurface(message);

          prevImg2 = -1;
        }

      if (powerReady) {
        powerReady = false;

        //cout<<"p: "<<powers[0][4]<<" "<<powers[0][5]<<" "<<powers[0][6]<<" "<<powers[0][7]<<endl;

        size_t fbufferSize = 26;
        float fbuffer[fbufferSize];
        /*for (size_t t=0; t<(sig_msg.size()/sizeof(float)); t++) {
          fbuffer[0] = buffer[t];
          fbuffer[1] = float(prevImg);
          fbuffer[2] = float(state);
          fbuffer[3] = float(clicked);
          //for (size_t tt=0; tt<(fbufferSize-4); tt++)
            fbuffer[4+tt] = powers[0][tt];
          cout<<"write"<<endl;
          fwrite(fbuffer, sizeof(float),fbufferSize , pFile);
          cout<<"wrote"<<endl;
        }*/
      }

      //clicked = false;
  }
cout<<"q2"<<endl;

  Mix_HaltMusic();

  helper1.join();

cout<<"q1"<<endl;
  //Release the surface
  for (size_t i=0; i<numImages; i++)
    SDL_FreeSurface(image[i]);

  SDL_FreeSurface(background);

  TTF_CloseFont(font);
  TTF_CloseFont(fontMed);
  TTF_CloseFont(fontBig);
  TTF_CloseFont(fontBigBack);
  TTF_Quit();

  Mix_CloseAudio();
  Mix_Quit();

  return 0;
}

