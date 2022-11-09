// $Id: RandLandau.cc,v 1.5 2010/06/16 17:24:53 garren Exp $
// -*- C++ -*-
//
// -----------------------------------------------------------------------
//                             HEP Random
//                          --- RandLandau ---
//                      class implementation file
// -----------------------------------------------------------------------

// =======================================================================
// M Fischler	  - Created 1/6/2000.
//
//		    The key transform() method uses the algorithm in CERNLIB.
//		    This is because I trust that RANLAN routine more than 
//		    I trust the Bukin-Grozina inverseLandau, which is not
//		    claimed to be better than 1% accurate.  
//
// M Fischler      - put and get to/from streams 12/13/04
// =======================================================================


#include "RandLandau.h"
#include <iostream>
#include <cmath>	// for std::log()

namespace CLHEP {

std::string RandLandau::name() const {return "RandLandau";}
HepRandomEngine & RandLandau::engine() {return *localEngine;}

RandLandau::~RandLandau() {
}

void RandLandau::shootArray( const int size, double* vect )
                            
{
  for( double* v = vect; v != vect + size; ++v )
    *v = shoot();
}

void RandLandau::shootArray( HepRandomEngine* anEngine,
                            const int size, double* vect )
{
  for( double* v = vect; v != vect + size; ++v )
    *v = shoot(anEngine);
}

void RandLandau::fireArray( const int size, double* vect)
{
  for( double* v = vect; v != vect + size; ++v )
    *v = fire();
}


//
// Table of values of inverse Landau, from r = .060 to .982
//

// Since all these are this is static to this compilation unit only, the 
// info is establised a priori and not at each invocation.

static const float TABLE_INTERVAL   = .001f;
static const int   TABLE_END        =  982;
static const float TABLE_MULTIPLIER = 1.0f/TABLE_INTERVAL;

// Here comes the big (4K bytes) table ---
//
// inverseLandau[ n ] = the inverse cdf at r = n*TABLE_INTERVAL = n/1000.
//
// Credit CERNLIB for these computations.
//
// This data is float because the algortihm does not benefit from further
// data accuracy.  The numbers below .006 or above .982 are moot since
// non-table-based methods are used below r=.007 and above .980.

static const float inverseLandau [TABLE_END+1] = {

0.0f,							     // .000
0.0f, 	    0.0f, 	0.0f, 	    0.0f, 	0.0f, 	     // .001 - .005
-2.244733f, -2.204365f, -2.168163f, -2.135219f, -2.104898f,  // .006 - .010
-2.076740f, -2.050397f, -2.025605f, -2.002150f, -1.979866f,
-1.958612f, -1.938275f, -1.918760f, -1.899984f, -1.881879f,  // .020
-1.864385f, -1.847451f, -1.831030f, -1.815083f, -1.799574f,
-1.784473f, -1.769751f, -1.755383f, -1.741346f, -1.727620f,  // .030
-1.714187f, -1.701029f, -1.688130f, -1.675477f, -1.663057f, 
-1.650858f, -1.638868f, -1.627078f, -1.615477f, -1.604058f,  // .040
-1.592811f, -1.581729f, -1.570806f, -1.560034f, -1.549407f,
-1.538919f, -1.528565f, -1.518339f, -1.508237f, -1.498254f,  // .050
-1.488386f, -1.478628f, -1.468976f, -1.459428f, -1.449979f,
-1.440626f, -1.431365f, -1.422195f, -1.413111f, -1.404112f,  // .060
-1.395194f, -1.386356f, -1.377594f, -1.368906f, -1.360291f,
-1.351746f, -1.343269f, -1.334859f, -1.326512f, -1.318229f,  // .070
-1.310006f, -1.301843f, -1.293737f, -1.285688f, -1.277693f,
-1.269752f, -1.261863f, -1.254024f, -1.246235f, -1.238494f,  // .080
-1.230800f, -1.223153f, -1.215550f, -1.207990f, -1.200474f,
-1.192999f, -1.185566f, -1.178172f, -1.170817f, -1.163500f,  // .090
-1.156220f, -1.148977f, -1.141770f, -1.134598f, -1.127459f,
-1.120354f, -1.113282f, -1.106242f, -1.099233f, -1.092255f,  // .100

-1.085306f, -1.078388f, -1.071498f, -1.064636f, -1.057802f,
-1.050996f, -1.044215f, -1.037461f, -1.030733f, -1.024029f,
-1.017350f, -1.010695f, -1.004064f, -.997456f,  -.990871f, 
-.984308f, -.977767f, -.971247f, -.964749f, -.958271f, 
-.951813f, -.945375f, -.938957f, -.932558f, -.926178f, 
-.919816f, -.913472f, -.907146f, -.900838f, -.894547f,
-.888272f, -.882014f, -.875773f, -.869547f, -.863337f, 
-.857142f, -.850963f, -.844798f, -.838648f, -.832512f, 
-.826390f, -.820282f, -.814187f, -.808106f, -.802038f, 
-.795982f, -.789940f, -.783909f, -.777891f, -.771884f, 	// .150
-.765889f, -.759906f, -.753934f, -.747973f, -.742023f, 
-.736084f, -.730155f, -.724237f, -.718328f, -.712429f, 
-.706541f, -.700661f, -.694791f, -.688931f, -.683079f, 
-.677236f, -.671402f, -.665576f, -.659759f, -.653950f, 
-.648149f, -.642356f, -.636570f, -.630793f, -.625022f, 
-.619259f, -.613503f, -.607754f, -.602012f, -.596276f, 
-.590548f, -.584825f, -.579109f, -.573399f, -.567695f, 
-.561997f, -.556305f, -.550618f, -.544937f, -.539262f,
-.533592f, -.527926f, -.522266f, -.516611f, -.510961f, 
-.505315f, -.499674f, -.494037f, -.488405f, -.482777f,	// .200

-.477153f, -.471533f, -.465917f, -.460305f, -.454697f, 
-.449092f, -.443491f, -.437893f, -.432299f, -.426707f, 
-.421119f, -.415534f, -.409951f, -.404372f, -.398795f, 
-.393221f, -.387649f, -.382080f, -.376513f, -.370949f, 
-.365387f, -.359826f, -.354268f, -.348712f, -.343157f, 
-.337604f, -.332053f, -.326503f, -.320955f, -.315408f,
-.309863f, -.304318f, -.298775f, -.293233f, -.287692f,
-.282152f, -.276613f, -.271074f, -.265536f, -.259999f, 
-.254462f, -.248926f, -.243389f, -.237854f, -.232318f, 
-.226783f, -.221247f, -.215712f, -.210176f, -.204641f, 	// .250
-.199105f, -.193568f, -.188032f, -.182495f, -.176957f, 
-.171419f, -.165880f, -.160341f, -.154800f, -.149259f,
-.143717f, -.138173f, -.132629f, -.127083f, -.121537f, 
-.115989f, -.110439f, -.104889f, -.099336f, -.093782f, 
-.088227f, -.082670f, -.077111f, -.071550f, -.065987f, 
-.060423f, -.054856f, -.049288f, -.043717f, -.038144f, 
-.032569f, -.026991f, -.021411f, -.015828f, -.010243f, 
-.004656f,  .000934f,  .006527f,  .012123f,  .017722f,
.023323f, .028928f,  .034535f,  .040146f,  .045759f, 
.051376f, .056997f,  .062620f,  .068247f,  .073877f,	// .300

.079511f,  .085149f,  .090790f,  .096435f,  .102083f,  
.107736f,  .113392f,  .119052f,  .124716f,  .130385f,  
.136057f,  .141734f,  .147414f,  .153100f,  .158789f,  
.164483f,  .170181f,  .175884f,  .181592f,  .187304f,  
.193021f,  .198743f,  .204469f,  .210201f,  .215937f,  
.221678f,  .227425f,  .233177f,  .238933f,  .244696f,  
.250463f,  .256236f,  .262014f,  .267798f,  .273587f,  
.279382f,  .285183f,  .290989f,  .296801f,  .302619f,  
.308443f,  .314273f,  .320109f,  .325951f,  .331799f,  
.337654f,  .343515f,  .349382f,  .355255f,  .361135f,	// .350  
.367022f,  .372915f,  .378815f,  .384721f,  .390634f,  
.396554f,  .402481f,  .408415f,  .414356f,  .420304f,
.426260f,  .432222f,  .438192f,  .444169f,  .450153f,  
.456145f,  .462144f,  .468151f,  .474166f,  .480188f,  
.486218f,  .492256f,  .498302f,  .504356f,  .510418f,  
.516488f,  .522566f,  .528653f,  .534747f,  .540850f,  
.546962f,  .553082f,  .559210f,  .565347f,  .571493f,  
.577648f,  .583811f,  .589983f,  .596164f,  .602355f,
.608554f,  .614762f,  .620980f,  .627207f,  .633444f,  
.639689f,  .645945f,  .652210f,  .658484f,  .664768f,	// .400

.671062f,  .677366f,  .683680f,  .690004f,  .696338f,  
.702682f,  .709036f,  .715400f,  .721775f,  .728160f,  
.734556f,  .740963f,  .747379f,  .753807f,  .760246f,  
.766695f,  .773155f,  .779627f,  .786109f,  .792603f,  
.799107f,  .805624f,  .812151f,  .818690f,  .825241f,  
.831803f,  .838377f,  .844962f,  .851560f,  .858170f,
.864791f,  .871425f,  .878071f,  .884729f,  .891399f,  
.898082f,  .904778f,  .911486f,  .918206f,  .924940f,  
.931686f,  .938446f,  .945218f,  .952003f,  .958802f,  
.965614f,  .972439f,  .979278f,  .986130f,  .992996f, 	// .450 
.999875f,  1.006769f, 1.013676f, 1.020597f, 1.027533f, 
1.034482f, 1.041446f, 1.048424f, 1.055417f, 1.062424f,
1.069446f, 1.076482f, 1.083534f, 1.090600f, 1.097681f, 
1.104778f, 1.111889f, 1.119016f, 1.126159f, 1.133316f, 
1.140490f, 1.147679f, 1.154884f, 1.162105f, 1.169342f, 
1.176595f, 1.183864f, 1.191149f, 1.198451f, 1.205770f, 
1.213105f, 1.220457f, 1.227826f, 1.235211f, 1.242614f, 
1.250034f, 1.257471f, 1.264926f, 1.272398f, 1.279888f,
1.287395f, 1.294921f, 1.302464f, 1.310026f, 1.317605f, 
1.325203f, 1.332819f, 1.340454f, 1.348108f, 1.355780f,	// .500

1.363472f, 1.371182f, 1.378912f, 1.386660f, 1.394429f, 
1.402216f, 1.410024f, 1.417851f, 1.425698f, 1.433565f, 
1.441453f, 1.449360f, 1.457288f, 1.465237f, 1.473206f, 
1.481196f, 1.489208f, 1.497240f, 1.505293f, 1.513368f, 
1.521465f, 1.529583f, 1.537723f, 1.545885f, 1.554068f, 
1.562275f, 1.570503f, 1.578754f, 1.587028f, 1.595325f,
1.603644f, 1.611987f, 1.620353f, 1.628743f, 1.637156f, 
1.645593f, 1.654053f, 1.662538f, 1.671047f, 1.679581f, 
1.688139f, 1.696721f, 1.705329f, 1.713961f, 1.722619f, 
1.731303f, 1.740011f, 1.748746f, 1.757506f, 1.766293f, 	// .550
1.775106f, 1.783945f, 1.792810f, 1.801703f, 1.810623f, 
1.819569f, 1.828543f, 1.837545f, 1.846574f, 1.855631f,
1.864717f, 1.873830f, 1.882972f, 1.892143f, 1.901343f, 
1.910572f, 1.919830f, 1.929117f, 1.938434f, 1.947781f, 
1.957158f, 1.966566f, 1.976004f, 1.985473f, 1.994972f, 
2.004503f, 2.014065f, 2.023659f, 2.033285f, 2.042943f, 
2.052633f, 2.062355f, 2.072110f, 2.081899f, 2.091720f, 
2.101575f, 2.111464f, 2.121386f, 2.131343f, 2.141334f,
2.151360f, 2.161421f, 2.171517f, 2.181648f, 2.191815f, 
2.202018f, 2.212257f, 2.222533f, 2.232845f, 2.243195f,	// .600

2.253582f, 2.264006f, 2.274468f, 2.284968f, 2.295507f, 
2.306084f, 2.316701f, 2.327356f, 2.338051f, 2.348786f, 
2.359562f, 2.370377f, 2.381234f, 2.392131f, 2.403070f, 
2.414051f, 2.425073f, 2.436138f, 2.447246f, 2.458397f, 
2.469591f, 2.480828f, 2.492110f, 2.503436f, 2.514807f, 
2.526222f, 2.537684f, 2.549190f, 2.560743f, 2.572343f,
2.583989f, 2.595682f, 2.607423f, 2.619212f, 2.631050f, 
2.642936f, 2.654871f, 2.666855f, 2.678890f, 2.690975f, 
2.703110f, 2.715297f, 2.727535f, 2.739825f, 2.752168f, 
2.764563f, 2.777012f, 2.789514f, 2.802070f, 2.814681f,	// .650 
2.827347f, 2.840069f, 2.852846f, 2.865680f, 2.878570f, 
2.891518f, 2.904524f, 2.917588f, 2.930712f, 2.943894f,
2.957136f, 2.970439f, 2.983802f, 2.997227f, 3.010714f,
3.024263f, 3.037875f, 3.051551f, 3.065290f, 3.079095f, 
3.092965f, 3.106900f, 3.120902f, 3.134971f, 3.149107f, 
3.163312f, 3.177585f, 3.191928f, 3.206340f, 3.220824f, 
3.235378f, 3.250005f, 3.264704f, 3.279477f, 3.294323f, 
3.309244f, 3.324240f, 3.339312f, 3.354461f, 3.369687f,
3.384992f, 3.400375f, 3.415838f, 3.431381f, 3.447005f, 
3.462711f, 3.478500f, 3.494372f, 3.510328f, 3.526370f,	// .700

3.542497f, 3.558711f, 3.575012f, 3.591402f, 3.607881f, 
3.624450f, 3.641111f, 3.657863f, 3.674708f, 3.691646f, 
3.708680f, 3.725809f, 3.743034f, 3.760357f, 3.777779f, 
3.795300f, 3.812921f, 3.830645f, 3.848470f, 3.866400f, 
3.884434f, 3.902574f, 3.920821f, 3.939176f, 3.957640f, 
3.976215f, 3.994901f, 4.013699f, 4.032612f, 4.051639f,
4.070783f, 4.090045f, 4.109425f, 4.128925f, 4.148547f,  
4.168292f, 4.188160f, 4.208154f, 4.228275f, 4.248524f, 
4.268903f, 4.289413f, 4.310056f, 4.330832f, 4.351745f, 
4.372794f, 4.393982f, 4.415310f, 4.436781f, 4.458395f, 
4.480154f, 4.502060f, 4.524114f, 4.546319f, 4.568676f,	// .750 
4.591187f, 4.613854f, 4.636678f, 4.659662f, 4.682807f,
4.706116f, 4.729590f, 4.753231f, 4.777041f, 4.801024f, 
4.825179f, 4.849511f, 4.874020f, 4.898710f, 4.923582f, 
4.948639f, 4.973883f, 4.999316f, 5.024942f, 5.050761f, 
5.076778f, 5.102993f, 5.129411f, 5.156034f, 5.182864f, 
5.209903f, 5.237156f, 5.264625f, 5.292312f, 5.320220f, 
5.348354f, 5.376714f, 5.405306f, 5.434131f, 5.463193f,
5.492496f, 5.522042f, 5.551836f, 5.581880f, 5.612178f, 
5.642734f, 5.673552f, 5.704634f, 5.735986f, 5.767610f,	// .800

5.799512f, 5.831694f, 5.864161f, 5.896918f, 5.929968f, 
5.963316f, 5.996967f, 6.030925f, 6.065194f, 6.099780f, 
6.134687f, 6.169921f, 6.205486f, 6.241387f, 6.277630f, 
6.314220f, 6.351163f, 6.388465f, 6.426130f, 6.464166f, 
6.502578f, 6.541371f, 6.580553f, 6.620130f, 6.660109f, 
6.700495f, 6.741297f, 6.782520f, 6.824173f, 6.866262f,
6.908795f, 6.951780f, 6.995225f, 7.039137f, 7.083525f, 
7.128398f, 7.173764f, 7.219632f, 7.266011f, 7.312910f, 
7.360339f, 7.408308f, 7.456827f, 7.505905f, 7.555554f, 
7.605785f, 7.656608f, 7.708035f, 7.760077f, 7.812747f, 	// .850
7.866057f, 7.920019f, 7.974647f, 8.029953f, 8.085952f, 
8.142657f, 8.200083f, 8.258245f, 8.317158f, 8.376837f,
8.437300f, 8.498562f, 8.560641f, 8.623554f, 8.687319f, 
8.751955f, 8.817481f, 8.883916f, 8.951282f, 9.019600f, 
9.088889f, 9.159174f, 9.230477f, 9.302822f, 9.376233f, 
9.450735f, 9.526355f, 9.603118f, 9.681054f, 9.760191f, 
 9.840558f,  9.922186f, 10.005107f, 10.089353f, 10.174959f,
10.261958f, 10.350389f, 10.440287f, 10.531693f, 10.624646f,
10.719188f, 10.815362f, 10.913214f, 11.012789f, 11.114137f,
11.217307f, 11.322352f, 11.429325f, 11.538283f, 11.649285f,	// .900

11.762390f, 11.877664f, 11.995170f, 12.114979f, 12.237161f, 
12.361791f, 12.488946f, 12.618708f, 12.751161f, 12.886394f, 
13.024498f, 13.165570f, 13.309711f, 13.457026f, 13.607625f, 
13.761625f, 13.919145f, 14.080314f, 14.245263f, 14.414134f, 
14.587072f, 14.764233f, 14.945778f, 15.131877f, 15.322712f, 
15.518470f, 15.719353f, 15.925570f, 16.137345f, 16.354912f, 
16.578520f, 16.808433f, 17.044929f, 17.288305f, 17.538873f, 
17.796967f, 18.062943f, 18.337176f, 18.620068f, 18.912049f, 
19.213574f, 19.525133f, 19.847249f, 20.180480f, 20.525429f, 
20.882738f, 21.253102f, 21.637266f, 22.036036f, 22.450278f, 	// .950
22.880933f, 23.329017f, 23.795634f, 24.281981f, 24.789364f, 
25.319207f, 25.873062f, 26.452634f, 27.059789f, 27.696581f,     // .960
28.365274f, 29.068370f, 29.808638f, 30.589157f, 31.413354f, 
32.285060f, 33.208568f, 34.188705f, 35.230920f, 36.341388f,     // .970
37.527131f, 38.796172f, 40.157721f, 41.622399f, 43.202525f, 
44.912465f, 46.769077f, 48.792279f, 51.005773f, 53.437996f,     // .980
56.123356f, 59.103894f, 					// .982

};  // End of the inverseLandau table


double RandLandau::transform (double r) {

  double  u = r * TABLE_MULTIPLIER; 
  int index = int(u);
  double du = u - index;

  // du is scaled such that the we dont have to multiply by TABLE_INTERVAL
  // when interpolating.

  // Five cases:
  // A) Between .070 and .800 the function is so smooth, straight
  //	linear interpolation is adequate.
  // B) Between .007 and .070, and between .800 and .980, quadratic
  //    interpolation is used.  This requires the same 4 points as
  //	a cubic spline (thus we need .006 and .981 and .982) but
  //	the quadratic interpolation is accurate enough and quicker.
  // C) Below .007 an asymptotic expansion for low negative lambda 
  //    (involving two logs) is used; there is a pade-style correction 
  //	factor.
  // D) Above .980, a simple pade approximation is made (asymptotic to
  //    1/(1-r)), but...
  // E) the coefficients in that pade are different above r=.999.

  if ( index >= 70 && index <= 800 ) {		// (A)

    double f0 = inverseLandau [index];
    double f1 = inverseLandau [index+1];
    return f0 + du * (f1 - f0);

  } else if ( index >= 7 && index <= 980 ) {	// (B)

    double f_1 = inverseLandau [index-1];
    double f0  = inverseLandau [index];
    double f1  = inverseLandau [index+1];
    double f2  = inverseLandau [index+2];

    return f0 + du * (f1 - f0 - .25*(1-du)* (f2 -f1 - f0 + f_1) );

  } else if ( index < 7 ) {			// (C)

    const double n0 =  0.99858950;
    const double n1 = 34.5213058;	const double d1 = 34.1760202;
    const double n2 = 17.0854528;	const double d2 =  4.01244582;

    double logr = std::log(r);
    double x    = 1/logr;
    double x2   = x*x;

    double pade = (n0 + n1*x + n2*x2) / (1.0 + d1*x + d2*x2);

    return ( - std::log ( -.91893853 - logr ) -1 ) * pade;

  } else if ( index <= 999 ) {			// (D)

    const double n0 =    1.00060006;
    const double n1 =  263.991156;	const double d1 =  257.368075;
    const double n2 = 4373.20068;	const double d2 = 3414.48018;

    double x = 1-r;
    double x2   = x*x;

    return (n0 + n1*x + n2*x2) / (x * (1.0 + d1*x + d2*x2));

  } else { 					// (E)

    const double n0 =      1.00001538;
    const double n1 =   6075.14119;	const double d1 =   6065.11919;
    const double n2 = 734266.409;	const double d2 = 694021.044;

    double x = 1-r;
    double x2   = x*x;

    return (n0 + n1*x + n2*x2) / (x * (1.0 + d1*x + d2*x2));

  }

} // transform()

std::ostream & RandLandau::put ( std::ostream & os ) const {
  int pr=os.precision(20);
  os << " " << name() << "\n";
  os.precision(pr);
  return os;
}

std::istream & RandLandau::get ( std::istream & is ) {
  std::string inName;
  is >> inName;
  if (inName != name()) {
    is.clear(std::ios::badbit | is.rdstate());
    std::cerr << "Mismatch when expecting to read state of a "
    	      << name() << " distribution\n"
	      << "Name found was " << inName
	      << "\nistream is left in the badbit state\n";
    return is;
  }
  return is;
}

}  // namespace CLHEP
