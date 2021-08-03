// Copyright 2019 Google LLC. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "../lib/mps_statespace.h"

#include "../lib/formux.h"
#include "gtest/gtest.h"

namespace qsim {

namespace mps {

namespace {

TEST(MPSStateSpaceTest, Create) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(5, 8);
  EXPECT_EQ(mps.num_qubits(), 5);
  EXPECT_EQ(mps.bond_dim(), 8);
}

TEST(MPSStateSpaceTest, BlockOffset) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(5, 8);
  for (unsigned i = 0; i < ss.Size(mps); ++i) {
    mps.get()[i] = i;
  }

  ASSERT_EQ(ss.GetBlockOffset(mps, 0), 0);
  ASSERT_EQ(ss.GetBlockOffset(mps, 1), 32);
  ASSERT_EQ(ss.GetBlockOffset(mps, 2), 256 + 32);
  ASSERT_EQ(ss.GetBlockOffset(mps, 3), 512 + 32);
  ASSERT_EQ(ss.GetBlockOffset(mps, 4), 768 + 32);
}

TEST(MPSStateSpaceTest, SetZero) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(4, 8);
  for (unsigned i = 0; i < ss.Size(mps); ++i) {
    mps.get()[i] = i;
  }
  ss.SetMPSZero(mps);
  for (unsigned i = 0; i < ss.Size(mps); ++i) {
    auto expected = 0.0;
    if (i == 0 || i == 32 || i == 256 + 32 || i == 512 + 32) {
      expected = 1;
    }
    EXPECT_NEAR(mps.get()[i], expected, 1e-5);
  }
}

TEST(MPSStateSpaceTest, Copy) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(10, 8);
  auto mps2 = ss.CreateMPS(10, 8);
  auto mps3 = ss.CreateMPS(10, 4);
  for (unsigned i = 0; i < ss.Size(mps); ++i) {
    mps.get()[i] = i;
  }
  ASSERT_FALSE(ss.CopyMPS(mps, mps3));
  ss.CopyMPS(mps, mps2);
  for (unsigned i = 0; i < ss.Size(mps); ++i) {
    EXPECT_NEAR(mps.get()[i], mps2.get()[i], 1e-5);
  }
}

TEST(MPSStateSpaceTest, ToWaveFunctionZero) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(2, 8);
  ss.SetMPSZero(mps);
  float *wf = new float[8];
  ss.ToWaveFunction(mps, wf);
  EXPECT_NEAR(wf[0], 1, 1e-5);
  for (unsigned i = 1; i < 8; ++i) {
    EXPECT_NEAR(wf[i], 0, 1e-5);
  }
  delete[](wf);
}

TEST(MPSStateSpaceTest, ToWaveFunction3) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(3, 4);

  // Set to highly entangled three qubit state.
  memset(mps.get(), 0, ss.RawSize(mps));
  mps.get()[0] = -0.6622649924853867;
  mps.get()[1] = -0.3110490936135273;
  mps.get()[2] = 0.681488760344724;
  mps.get()[3] = -0.015052443773988289;
  mps.get()[8] = -0.537553225765131;
  mps.get()[9] = 0.4191539781192369;
  mps.get()[10] = -0.31650636199260096;
  mps.get()[11] = 0.659674338467379;
  mps.get()[16] = -0.21720151603221893;
  mps.get()[17] = 0.5354822278022766;
  mps.get()[18] = -0.24278810620307922;
  mps.get()[19] = 0.12074445933103561;
  mps.get()[24] = -0.10164494812488556;
  mps.get()[25] = -0.6021595597267151;
  mps.get()[26] = 0.49309641122817993;
  mps.get()[27] = 0.05576712265610695;
  mps.get()[32] = -0.3956003189086914;
  mps.get()[33] = -0.1778077632188797;
  mps.get()[34] = -0.1472112536430359;
  mps.get()[35] = 0.7757846117019653;
  mps.get()[40] = 0.3030144274234772;
  mps.get()[41] = -0.11498478055000305;
  mps.get()[42] = 0.06491414457559586;
  mps.get()[43] = -0.22911544144153595;
  mps.get()[80] = 0.5297775864601135;
  mps.get()[81] = 0.0;
  mps.get()[82] = -0.6799570918083191;
  mps.get()[83] = 0.41853320598602295;
  mps.get()[84] = -0.23835298418998718;
  mps.get()[85] = 0.0;
  mps.get()[86] = -0.13468137383460999;
  mps.get()[87] = 0.0829002782702446;

  // Check that the following transformation is carried out:
  // wf = einsum('ij,jkl,lm->ikm', *blocks)
  float *wf = new float[32];
  ss.ToWaveFunction(mps, wf);
  EXPECT_NEAR(wf[0], -0.005946025252342224, 1e-4);
  EXPECT_NEAR(wf[1], -0.3386073410511017, 1e-4);
  EXPECT_NEAR(wf[2], 0.08402486890554428, 1e-4);
  EXPECT_NEAR(wf[3], 0.2276899814605713, 1e-4);
  EXPECT_NEAR(wf[4], 0.10889682918787003, 1e-4);
  EXPECT_NEAR(wf[5], 0.26689958572387695, 1e-4);
  EXPECT_NEAR(wf[6], -0.13812999427318573, 1e-4);
  EXPECT_NEAR(wf[7], -0.17624962329864502, 1e-4);
  EXPECT_NEAR(wf[8], 0.16325148940086365, 1e-4);
  EXPECT_NEAR(wf[9], -0.18776941299438477, 1e-4);
  EXPECT_NEAR(wf[10], 0.24669288098812103, 1e-4);
  EXPECT_NEAR(wf[11], 0.48989138007164, 1e-4);
  EXPECT_NEAR(wf[12], 0.18966005742549896, 1e-4);
  EXPECT_NEAR(wf[13], 0.204482764005661, 1e-4);
  EXPECT_NEAR(wf[14], -0.41462600231170654, 1e-4);
  EXPECT_NEAR(wf[15], -0.28409692645072937, 1e-4);
  delete[](wf);
}

TEST(MPSStateSpaceTest, ToWaveFunction5) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(5, 4);

  // Set to highly entangled five qubit state.
  memset(mps.get(), 0, ss.RawSize(mps));
  mps.get()[0] = -0.7942508170779394;
  mps.get()[1] = -0.08353012422743371;
  mps.get()[2] = -0.5956724071158231;
  mps.get()[3] = -0.0858062557546432;
  mps.get()[8] = 0.6008886732655473;
  mps.get()[9] = -0.03348407052200576;
  mps.get()[10] = -0.7985104374257858;
  mps.get()[11] = 0.013883241380578323;
  mps.get()[16] = -0.31778309393577153;
  mps.get()[17] = 0.08129081012436856;
  mps.get()[18] = -0.17084936092778547;
  mps.get()[19] = -0.02218120545861387;
  mps.get()[20] = -0.4708915300196999;
  mps.get()[21] = 0.5554105084817618;
  mps.get()[22] = 0.4771044130233731;
  mps.get()[23] = 0.3238455071330493;
  mps.get()[24] = -0.255477406163936;
  mps.get()[25] = 0.4374921994586982;
  mps.get()[26] = -0.5501925628308599;
  mps.get()[27] = 0.16130434535302918;
  mps.get()[28] = -0.22510697789781603;
  mps.get()[29] = 0.05157889931677101;
  mps.get()[30] = -0.5462643594366281;
  mps.get()[31] = -0.2507242261622358;
  mps.get()[32] = -0.257977790582352;
  mps.get()[33] = -0.11224285788942705;
  mps.get()[34] = -0.29538188714282193;
  mps.get()[35] = -0.38072576149146387;
  mps.get()[36] = 0.6001487220096956;
  mps.get()[37] = 0.1913733701851922;
  mps.get()[38] = -0.23636184929019038;
  mps.get()[39] = 0.4857749031783798;
  mps.get()[40] = 0.10130150715330866;
  mps.get()[41] = -0.7391377306145324;
  mps.get()[42] = -0.44876238752931974;
  mps.get()[43] = 0.4560672064449336;
  mps.get()[44] = 0.028438967271747218;
  mps.get()[45] = 0.13724346784210212;
  mps.get()[46] = 0.003584017578785237;
  mps.get()[47] = -0.11987932710918753;
  mps.get()[80] = 0.40840303886247986;
  mps.get()[81] = 0.0;
  mps.get()[82] = 0.07592798473660406;
  mps.get()[83] = 0.7192043122227202;
  mps.get()[84] = -0.1351739336607331;
  mps.get()[85] = 0.31415911338868924;
  mps.get()[86] = -0.2543437131216091;
  mps.get()[87] = 0.1901822451454096;
  mps.get()[88] = -0.49494962111198254;
  mps.get()[89] = 0.3938336604677486;
  mps.get()[90] = 0.12794790638132017;
  mps.get()[91] = 0.23588305655979178;
  mps.get()[92] = -0.08352038306191087;
  mps.get()[93] = 0.4006572203199725;
  mps.get()[94] = 0.36886860844013736;
  mps.get()[95] = -0.1586842041599526;
  mps.get()[96] = 0.1834561393756626;
  mps.get()[97] = 0.0;
  mps.get()[98] = 0.19628042396288672;
  mps.get()[99] = -0.40233821643752055;
  mps.get()[100] = -0.5974332727264484;
  mps.get()[101] = 0.19287040617030263;
  mps.get()[102] = 0.1053276514717207;
  mps.get()[103] = 0.016804190083581708;
  mps.get()[104] = -0.263327065774291;
  mps.get()[105] = 0.43922624365712193;
  mps.get()[106] = 0.10968978610217328;
  mps.get()[107] = -0.19665026336865873;
  mps.get()[108] = -0.06004766570619344;
  mps.get()[109] = -0.028059745847255218;
  mps.get()[110] = -0.24855708157570078;
  mps.get()[111] = 0.5751767140835897;
  mps.get()[112] = 0.25199694912392945;
  mps.get()[113] = 0.0;
  mps.get()[114] = -0.05739258827501658;
  mps.get()[115] = -0.30245742194728265;
  mps.get()[116] = 0.13607116127541907;
  mps.get()[117] = 0.17118330269631235;
  mps.get()[118] = -0.22592603732824876;
  mps.get()[119] = 0.27239431845297707;
  mps.get()[120] = 0.01047777976886481;
  mps.get()[121] = -0.21390579587098454;
  mps.get()[122] = 0.020345493365053653;
  mps.get()[123] = -0.15489716040222756;
  mps.get()[124] = -0.2920457586238394;
  mps.get()[125] = 0.32807225065061896;
  mps.get()[126] = -0.22441139544567443;
  mps.get()[127] = -0.15516902178850114;
  mps.get()[128] = 0.1303815766294433;
  mps.get()[129] = 0.0;
  mps.get()[130] = 0.09443469130980126;
  mps.get()[131] = 0.09749552478738743;
  mps.get()[132] = 0.07115934313302229;
  mps.get()[133] = 0.07172860752123576;
  mps.get()[134] = 0.35262084813015576;
  mps.get()[135] = 0.05559150244274026;
  mps.get()[136] = 0.05585983377252125;
  mps.get()[137] = -0.08787607283694769;
  mps.get()[138] = -0.02888091663074432;
  mps.get()[139] = 0.12419549395557358;
  mps.get()[140] = -0.24857309811183348;
  mps.get()[141] = -0.06536920925603362;
  mps.get()[142] = -0.026777844823335055;
  mps.get()[143] = 0.07798739264017497;
  mps.get()[144] = -0.4022885859012604;
  mps.get()[145] = 0.529089629650116;
  mps.get()[146] = 0.021047838032245636;
  mps.get()[147] = 0.11089000850915909;
  mps.get()[152] = -0.11812450736761093;
  mps.get()[153] = -0.3155742883682251;
  mps.get()[154] = -0.025639047846198082;
  mps.get()[155] = 0.5808156132698059;
  mps.get()[160] = 0.0904598981142044;
  mps.get()[161] = -0.03687569126486778;
  mps.get()[162] = 0.4893633723258972;
  mps.get()[163] = 0.2733270823955536;
  mps.get()[168] = 0.2756871283054352;
  mps.get()[169] = -0.2685239017009735;
  mps.get()[170] = 0.0703665167093277;
  mps.get()[171] = -0.11739754676818848;
  mps.get()[176] = -0.040402818471193314;
  mps.get()[177] = 0.024999519810080528;
  mps.get()[178] = 0.2142343968153;
  mps.get()[179] = 0.3487721085548401;
  mps.get()[184] = -0.38712623715400696;
  mps.get()[185] = 0.2719499170780182;
  mps.get()[186] = -0.28398218750953674;
  mps.get()[187] = -0.12957964837551117;
  mps.get()[192] = -0.16253285109996796;
  mps.get()[193] = 0.1666962057352066;
  mps.get()[194] = 0.029656991362571716;
  mps.get()[195] = -0.07687799632549286;
  mps.get()[200] = 0.05283937603235245;
  mps.get()[201] = 0.06291946768760681;
  mps.get()[202] = 0.01979890652000904;
  mps.get()[203] = -0.21019403636455536;
  mps.get()[208] = -0.7146716713905334;
  mps.get()[209] = 0.0;
  mps.get()[210] = 0.3957919478416443;
  mps.get()[211] = -0.1956116110086441;
  mps.get()[212] = -0.28512677550315857;
  mps.get()[213] = 0.0;
  mps.get()[214] = -0.41377660632133484;
  mps.get()[215] = 0.20450012385845184;

  // Check that the following transformation is carried out:
  // wf = einsum('ij,jkl,lmn,nop,pq->ikmoq', *blocks)
  float *wf = new float[128];
  ss.ToWaveFunction(mps, wf);
  EXPECT_NEAR(wf[0], 0.0027854256331920624, 1e-4);
  EXPECT_NEAR(wf[1], -0.14140120148658752, 1e-4);
  EXPECT_NEAR(wf[2], 0.030212486162781715, 1e-4);
  EXPECT_NEAR(wf[3], 0.05706779286265373, 1e-4);
  EXPECT_NEAR(wf[4], -0.09160802513360977, 1e-4);
  EXPECT_NEAR(wf[5], -0.05029388517141342, 1e-4);
  EXPECT_NEAR(wf[6], -0.06708981841802597, 1e-4);
  EXPECT_NEAR(wf[7], -0.06412483751773834, 1e-4);
  EXPECT_NEAR(wf[8], -0.0774611234664917, 1e-4);
  EXPECT_NEAR(wf[9], 0.27072837948799133, 1e-4);
  EXPECT_NEAR(wf[10], -0.003501715138554573, 1e-4);
  EXPECT_NEAR(wf[11], -0.2887609601020813, 1e-4);
  EXPECT_NEAR(wf[12], 0.016577117145061493, 1e-4);
  EXPECT_NEAR(wf[13], 0.1369006335735321, 1e-4);
  EXPECT_NEAR(wf[14], 0.08254759013652802, 1e-4);
  EXPECT_NEAR(wf[15], 0.20499306917190552, 1e-4);
  EXPECT_NEAR(wf[16], 0.17876368761062622, 1e-4);
  EXPECT_NEAR(wf[17], -0.02268427424132824, 1e-4);
  EXPECT_NEAR(wf[18], 0.05583261698484421, 1e-4);
  EXPECT_NEAR(wf[19], 0.10677587240934372, 1e-4);
  EXPECT_NEAR(wf[20], 0.018177300691604614, 1e-4);
  EXPECT_NEAR(wf[21], 0.26146093010902405, 1e-4);
  EXPECT_NEAR(wf[22], -0.19240343570709229, 1e-4);
  EXPECT_NEAR(wf[23], -0.12706275284290314, 1e-4);
  EXPECT_NEAR(wf[24], 0.1699770838022232, 1e-4);
  EXPECT_NEAR(wf[25], 0.26863881945610046, 1e-4);
  EXPECT_NEAR(wf[26], -0.10701578855514526, 1e-4);
  EXPECT_NEAR(wf[27], -0.03779822587966919, 1e-4);
  EXPECT_NEAR(wf[28], -0.06767062097787857, 1e-4);
  EXPECT_NEAR(wf[29], 0.05558207631111145, 1e-4);
  EXPECT_NEAR(wf[30], 0.06148408725857735, 1e-4);
  EXPECT_NEAR(wf[31], -0.03445826843380928, 1e-4);
  EXPECT_NEAR(wf[32], -0.018822386860847473, 1e-4);
  EXPECT_NEAR(wf[33], -0.007597930729389191, 1e-4);
  EXPECT_NEAR(wf[34], -0.0027186088263988495, 1e-4);
  EXPECT_NEAR(wf[35], 0.003467019647359848, 1e-4);
  EXPECT_NEAR(wf[36], -0.26657143235206604, 1e-4);
  EXPECT_NEAR(wf[37], -0.029667221009731293, 1e-4);
  EXPECT_NEAR(wf[38], 0.1857101023197174, 1e-4);
  EXPECT_NEAR(wf[39], -0.055891260504722595, 1e-4);
  EXPECT_NEAR(wf[40], -0.060019031167030334, 1e-4);
  EXPECT_NEAR(wf[41], 0.06737485527992249, 1e-4);
  EXPECT_NEAR(wf[42], -0.038918495178222656, 1e-4);
  EXPECT_NEAR(wf[43], -0.045035410672426224, 1e-4);
  EXPECT_NEAR(wf[44], -0.1498071402311325, 1e-4);
  EXPECT_NEAR(wf[45], -0.15015973150730133, 1e-4);
  EXPECT_NEAR(wf[46], 0.11186741292476654, 1e-4);
  EXPECT_NEAR(wf[47], 0.057124655693769455, 1e-4);
  EXPECT_NEAR(wf[48], 0.16711947321891785, 1e-4);
  EXPECT_NEAR(wf[49], 0.2237841784954071, 1e-4);
  EXPECT_NEAR(wf[50], 0.20187999308109283, 1e-4);
  EXPECT_NEAR(wf[51], 0.02212279662489891, 1e-4);
  EXPECT_NEAR(wf[52], 0.07793829590082169, 1e-4);
  EXPECT_NEAR(wf[53], -0.11144962906837463, 1e-4);
  EXPECT_NEAR(wf[54], 0.11177311837673187, 1e-4);
  EXPECT_NEAR(wf[55], -0.02343379706144333, 1e-4);
  EXPECT_NEAR(wf[56], -0.08419902622699738, 1e-4);
  EXPECT_NEAR(wf[57], 0.029235713183879852, 1e-4);
  EXPECT_NEAR(wf[58], 0.12327411770820618, 1e-4);
  EXPECT_NEAR(wf[59], 0.059630997478961945, 1e-4);
  EXPECT_NEAR(wf[60], -0.04118343070149422, 1e-4);
  EXPECT_NEAR(wf[61], -0.14594365656375885, 1e-4);
  EXPECT_NEAR(wf[62], -0.11883178353309631, 1e-4);
  EXPECT_NEAR(wf[63], 0.1824525147676468, 1e-4);
  delete[](wf);
}

TEST(MPSStateSpaceTest, InnerProduct4) {
  auto ss = MPSStateSpace<For, float>(1);
  auto mps = ss.CreateMPS(4, 4);
  auto mps2 = ss.CreateMPS(4, 4);

  // Set to highly entangled four qubit state.
  memset(mps.get(), 0, ss.RawSize(mps));
  memset(mps2.get(), 0, ss.RawSize(mps2));
  mps.get()[0] = -0.916497861382668;
  mps.get()[1] = -0.0774770100056814;
  mps.get()[2] = -0.3905530508872181;
  mps.get()[3] = -0.038695257453215746;
  mps.get()[8] = 0.39242052841785685;
  mps.get()[9] = 0.005926209849421993;
  mps.get()[10] = -0.9193660433571464;
  mps.get()[11] = -0.027148413259157553;
  mps.get()[16] = -0.086494587815096;
  mps.get()[17] = -0.5161113650581821;
  mps.get()[18] = -0.3716843459879704;
  mps.get()[19] = -0.4149275842783076;
  mps.get()[20] = 0.3475684513942029;
  mps.get()[21] = -0.33731825676083277;
  mps.get()[22] = 0.03531924421420863;
  mps.get()[23] = 0.4242625462238508;
  mps.get()[24] = 0.1548611214464985;
  mps.get()[25] = -0.1629745551510658;
  mps.get()[26] = -0.3054123508603024;
  mps.get()[27] = 0.40742455983835185;
  mps.get()[28] = 0.051375370785247995;
  mps.get()[29] = 0.6739332289909812;
  mps.get()[30] = 0.1957074863128766;
  mps.get()[31] = 0.4416548486767887;
  mps.get()[32] = -0.4188134561454451;
  mps.get()[33] = -0.314779963690704;
  mps.get()[34] = 0.594871513074914;
  mps.get()[35] = 0.1253634938807484;
  mps.get()[36] = -0.3274468059583836;
  mps.get()[37] = -0.0033649355295961303;
  mps.get()[38] = -0.19836336090039158;
  mps.get()[39] = 0.4575368665727339;
  mps.get()[40] = -0.4319730509600821;
  mps.get()[41] = 0.46315571812161255;
  mps.get()[42] = -0.177092245869463;
  mps.get()[43] = 0.17165251096868606;
  mps.get()[44] = 0.4478329658040191;
  mps.get()[45] = 0.028284989048036946;
  mps.get()[46] = -0.5484962316855873;
  mps.get()[47] = 0.1893602226102037;
  mps.get()[80] = 0.5355256929496379;
  mps.get()[81] = 0.0;
  mps.get()[82] = -0.82749362448062;
  mps.get()[83] = 0.02904044194569624;
  mps.get()[84] = 0.0;
  mps.get()[85] = 3.1712172333499e-18;
  mps.get()[88] = 0.08673107202101067;
  mps.get()[89] = -0.26957426786565664;
  mps.get()[90] = 0.10136853320009953;
  mps.get()[91] = -0.16847174758615416;
  mps.get()[96] = 0.7256882794862672;
  mps.get()[97] = 0.0;
  mps.get()[98] = 0.49992356328580695;
  mps.get()[99] = -0.07465158451531788;
  mps.get()[100] = 0.0;
  mps.get()[101] = -2.73164461529292e-18;
  mps.get()[104] = -0.11096745459559126;
  mps.get()[105] = -0.11248021223295962;
  mps.get()[106] = -0.015939524128979008;
  mps.get()[107] = -0.04834685546748854;
  mps.get()[112] = -0.09137803308510727;
  mps.get()[113] = 0.0;
  mps.get()[114] = 0.041828533843678406;
  mps.get()[115] = -0.055516336152773675;
  mps.get()[116] = -1.7346894763697954e-17;
  mps.get()[117] = -7.589266459117856e-18;
  mps.get()[120] = -0.06982795298266756;
  mps.get()[121] = -0.2607434376975409;
  mps.get()[122] = 0.04055209540168665;
  mps.get()[123] = -0.0998159882317749;
  mps.get()[128] = -0.0013533723870614552;
  mps.get()[129] = 0.0;
  mps.get()[130] = 0.0030153696871580518;
  mps.get()[131] = -0.0007536486755610136;
  mps.get()[132] = 1.3706310124710953e-17;
  mps.get()[133] = 5.271657740273443e-18;
  mps.get()[136] = 0.009007639720827557;
  mps.get()[137] = 0.01160295765732885;
  mps.get()[138] = -0.002650020644033365;
  mps.get()[139] = -0.0347660454843333;
  mps.get()[144] = 0.7934826958343173;
  mps.get()[145] = 0.2097612636620367;
  mps.get()[146] = 0.40098701589649566;
  mps.get()[147] = 0.06292071832569604;
  mps.get()[148] = 0.17644861904250161;
  mps.get()[149] = 0.02508862414716359;
  mps.get()[150] = -0.36011160812021614;
  mps.get()[151] = -0.013850284789667294;
  
  // Set to slightly different four qubit state.
  mps2.get()[0] = -0.916497861382668;
  mps2.get()[1] = -0.0774770100056814;
  mps2.get()[2] = -0.3905530508872181;
  mps2.get()[3] = -0.038695257453215746;
  mps2.get()[8] = 0.39242052841785685;
  mps2.get()[9] = 0.005926209849421993;
  mps2.get()[10] = -0.9193660433571464;
  mps2.get()[11] = -0.027148413259157553;
  mps2.get()[16] = -0.38520893663443145;
  mps2.get()[17] = -0.08313325347846491;
  mps2.get()[18] = 0.37387886041396534;
  mps2.get()[19] = 0.7642074712965752;
  mps2.get()[20] = -0.27881372303099244;
  mps2.get()[21] = 0.1474857317523121;
  mps2.get()[22] = -0.1410007330015855;
  mps2.get()[23] = -0.039168047247753496;
  mps2.get()[24] = -0.0590745502568466;
  mps2.get()[25] = 0.11761847202902623;
  mps2.get()[26] = 0.11269537822823146;
  mps2.get()[27] = -0.3086460273383095;
  mps2.get()[28] = -0.6327237072338668;
  mps2.get()[29] = -0.28314375337094555;
  mps2.get()[30] = -0.15819977431031695;
  mps2.get()[31] = -0.6075990707063283;
  mps2.get()[32] = 0.8082960956126871;
  mps2.get()[33] = 0.4057876159937702;
  mps2.get()[34] = 0.12408608368116913;
  mps2.get()[35] = 0.3850457786727492;
  mps2.get()[36] = -0.029431664112584088;
  mps2.get()[37] = -0.08738621657419658;
  mps2.get()[38] = -0.039495020284007906;
  mps2.get()[39] = -0.0909603999525164;
  mps2.get()[40] = 0.0164446476145238;
  mps2.get()[41] = 0.095406687086266;
  mps2.get()[42] = 0.015460689255213836;
  mps2.get()[43] = -0.06589597358749627;
  mps2.get()[44] = -0.5539889126392532;
  mps2.get()[45] = -0.32341135258910775;
  mps2.get()[46] = 0.1325213431271281;
  mps2.get()[47] = 0.7463144784082719;
  mps2.get()[80] = 0.3879496172458074;
  mps2.get()[81] = 0.0;
  mps2.get()[82] = 0.7012769606101399;
  mps2.get()[83] = -0.12695868636166885;
  mps2.get()[84] = 1.1103700291614824e-16;
  mps2.get()[85] = 4.629873324419367e-18;
  mps2.get()[88] = 0.36012400471668854;
  mps2.get()[89] = 0.11784653120900945;
  mps2.get()[90] = -0.5483875743376463;
  mps2.get()[91] = -0.1637597971215351;
  mps2.get()[92] = 4.775145770909058e-18;
  mps2.get()[93] = 1.79364974950039e-17;
  mps2.get()[96] = -0.640372512495744;
  mps2.get()[97] = 0.0;
  mps2.get()[98] = 0.2548579767415688;
  mps2.get()[99] = -0.034454109442162505;
  mps2.get()[100] = 9.488019311652e-17;
  mps2.get()[101] = 1.0530014819474617e-17;
  mps2.get()[104] = 0.44752468366493875;
  mps2.get()[105] = -0.12895732984521566;
  mps2.get()[106] = 0.1804908199125375;
  mps2.get()[107] = -0.11201596042542786;
  mps2.get()[108] = -1.4515782829099415e-19;
  mps2.get()[109] = 4.5471437738577115e-18;
  mps2.get()[112] = -0.16454563839144662;
  mps2.get()[113] = 0.0;
  mps2.get()[114] = -0.024056710061469547;
  mps2.get()[115] = -0.1203420866582053;
  mps2.get()[116] = -3.1207550335607834e-17;
  mps2.get()[117] = -1.1028836460006021e-17;
  mps2.get()[120] = -0.13538852421270092;
  mps2.get()[121] = 0.17274307394393765;
  mps2.get()[122] = -0.15244639495683454;
  mps2.get()[123] = -0.06245206468145512;
  mps2.get()[124] = -1.5247965666831198e-18;
  mps2.get()[125] = 1.4070202389092805e-18;
  mps2.get()[128] = 0.03453277422180958;
  mps2.get()[129] = 0.0;
  mps2.get()[130] = -0.02287709221765174;
  mps2.get()[131] = -0.06623554376900025;
  mps2.get()[132] = -2.100635435828622e-17;
  mps2.get()[133] = 8.534146150309484e-19;
  mps2.get()[136] = 0.03199717502952966;
  mps2.get()[137] = 0.03835220263481228;
  mps2.get()[138] = 0.05616254494558428;
  mps2.get()[139] = -0.05491726676672418;
  mps2.get()[140] = -1.6687930640538633e-18;
  mps2.get()[141] = -1.0473130086052244e-19;
  mps2.get()[144] = 0.7934826958343173;
  mps2.get()[145] = 0.2097612636620367;
  mps2.get()[146] = 0.40098701589649566;
  mps2.get()[147] = 0.06292071832569604;
  mps2.get()[148] = 0.17644861904250161;
  mps2.get()[149] = 0.02508862414716359;
  mps2.get()[150] = -0.36011160812021614;
  mps2.get()[151] = -0.013850284789667294;

  // Computes the following contraction:
  //      +---+ +---+ +---+ +---+
  // mps2 | 0 +-+ 1 +-+ 2 +-+ 3 |
  //      +-+-+ +-+-+ +-+-+ +-+-+
  //        |     |     |     |
  //        |     |     |     |
  //      +-+-+ +-+-+ +-+-+ +-+-+
  // mps  | 0 +-+ 1 +-+ 2 +-+ 3 |
  //      +---+ +---+ +---+ +---+
  //
  // 0.5524505270081406+0.2471560922399374j
  auto r = ss.InnerProduct(mps, mps2);
  EXPECT_NEAR(r.real(), 0.5524, 1e-4);
  EXPECT_NEAR(r.imag(), 0.2471, 1e-4);
}

}  // namespace
}  // namespace mps
}  // namespace qsim

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}