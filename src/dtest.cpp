#if !defined(ARMA_64BIT_WORD)  
  #define ARMA_64BIT_WORD  
#endif
#define INFINITY
#include "createSpikeNet.h"
#include "runSpikeNet.h"
#include <typeinfo>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <armadillo>
using namespace std;
using namespace arma;

mat uhlenbeck(float theta, float mu, float sigma, float dt, int T)
{
  vec t = arma::linspace<vec>(0, T*dt, T);
  mat x(1, T); x.fill(mu);

  for (int i = 1; i < T; i++)
  {
    x(i) = theta*(mu - x(i-1))*dt + sigma*randn();
  }

  return x;
}

int main(){
  
  arma_rng::set_seed(42);
  
  // LIF Parameters
  float vth = -40.0;
  float vreset = -65.0;
  float vinf = -39.0;
  float tref = .002; //refractory time
  float tm = .01; //membrane relaxation
  float td = .03; //decrease
  float tr = .002; //rise

  // Network parameters
  int N = 2000; //neurons
  float p = .1; //sparsity
  int nIn = 3; //inputs
  int nOut = 2; //outputs
  float G = .05;
  float Q = 20.0;

  // Initial values
  sp_mat w_sp = sprandn(N, N, p)/sqrt(N*p);
  string loadPath = "/home/neurociencia/disc/G_005_Q_20_l_4e5/";
  string savePath = "/home/neurociencia/disc/G_005_Q_20_l_4e5/test11/";
  mat w0, wOut, wIn, wFb;
  w0.load(loadPath + "w_init.dat", raw_ascii);
  wOut.load(loadPath + "wOut10.dat", raw_ascii);
  wIn.load(loadPath + "wIn_init.dat", raw_ascii);
  wFb.load(loadPath + "wFb_init.dat", raw_ascii);
  
  vec v = (vth - vreset)*arma::randu<vec>(N) + vreset; // uniformly distributed within the linear regime
  vec r = arma::zeros<vec>(N);
  vec h = arma::zeros<vec>(N);

  vec dv = v;
  vec dr = r;
  vec dh = h;

  //Integration parameters
  int T = 140000;
  float dt = 5e-5;
  float totalTime = T*dt;

  //Auxiliary vars
  mat spikes = arma::zeros<mat>(T, N); //keep track of spikes
  mat ref = arma::zeros<vec>(N); //which neurons are in refractory period
  
  //FORCE parameters
  float trainRate = 4e5; //the smaller the faster the training
  mat P; P = eye(N,N)/trainRate;
  mat err = arma::zeros<mat>(nOut, N);
  int trainStep = (int)INFINITY;
  int trainStart = 0;
  int trainStop = T;
  int saveRate = 100;

  _Net myNet = createSpikeNet(vth, vreset, vinf, tref, tm, td, tr, N, p, nIn, nOut, G, Q, w0, wIn, wOut, wFb, v, r, h, dv, dr, dh, spikes, ref, P, err);

  // Read input and target from file
  ivec trials;
  trials.load("/home/neurociencia/disc/trials.dat", raw_ascii);

  mat trial;
  mat inp = arma::zeros<mat>(nIn, T);
  mat tgt = arma::zeros<mat>(nOut, T);

  // True only for checking chaotic behaviour by removing one spike
  bool spikeTest = false;

  myNet = runSpikeNet(myNet, arma::zeros<mat>(nIn, 10000), arma::zeros<mat>(nOut, 10000), dt, (int)INFINITY, 0, 10000, trainRate, (int)INFINITY, 0, (int)INFINITY, savePath, false, 0); //run for 0.5 seconds to reach equilibrium

  for (int j = 19; j < trials.n_elem; j++)
  {
    trial.load("/home/neurociencia/disc/trial" + toString(arma::as_scalar(trials.row(j))) + ".dat");
    inp = trial.cols(0,2);
    tgt = trial.cols(3,4);
    inp = inp.t();
    tgt = tgt.t();

    myNet = runSpikeNet(myNet, inp, tgt, dt, trainStep, trainStart, trainStop, trainRate, saveRate, j, (int)INFINITY, savePath, spikeTest, 0);

    w0 = myNet.w0;
    w0.save(savePath + "w" + toString(j) + ".dat", raw_ascii);

    wOut = myNet.wOut;
    wOut.save(savePath + "wOut" + toString(j) + ".dat", raw_ascii);
  
  }

  return 0;
}
