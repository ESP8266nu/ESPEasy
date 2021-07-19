#include "ESPEasy_common.h"

#include "src/Globals/Plugins.h"
#include "src/Globals/Settings.h"

#include "src/Helpers/Misc.h"


// ********************************************************************************
// Initialize all plugins that where defined earlier
// and initialize the function call pointer into the plugin array
// ********************************************************************************


// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDPLUGIN(NNN) if (addPlugin(PLUGIN_ID_##NNN, x)) Plugin_ptr[x++] = &Plugin_##NNN;
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

void PluginInit(void)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  DeviceIndex_to_Plugin_id[PLUGIN_MAX] = INVALID_PLUGIN_ID;
  // Clear pointer table for all plugins
  for (deviceIndex_t x = 0; x < PLUGIN_MAX; x++)
  {
    Plugin_ptr[x] = nullptr;
    DeviceIndex_to_Plugin_id[x] = INVALID_PLUGIN_ID;
    // Do not initialize Plugin_id_to_DeviceIndex[x] to an invalid value. (it is map)
  }
  uint32_t x = 0; // Used in ADDPLUGIN macro

#ifdef PLUGIN_001
  ADDPLUGIN(001)
#endif

#ifdef PLUGIN_002
  ADDPLUGIN(002)
#endif

#ifdef PLUGIN_003
  ADDPLUGIN(003)
#endif

#ifdef PLUGIN_004
  ADDPLUGIN(004)
#endif

#ifdef PLUGIN_005
  ADDPLUGIN(005)
#endif

#ifdef PLUGIN_006
  ADDPLUGIN(006)
#endif

#ifdef PLUGIN_007
  ADDPLUGIN(007)
#endif

#ifdef PLUGIN_008
  ADDPLUGIN(008)
#endif

#ifdef PLUGIN_009
  ADDPLUGIN(009)
#endif

#ifdef PLUGIN_010
  ADDPLUGIN(010)
#endif

#ifdef PLUGIN_011
  ADDPLUGIN(011)
#endif

#ifdef PLUGIN_012
  ADDPLUGIN(012)
#endif

#ifdef PLUGIN_013
  ADDPLUGIN(013)
#endif

#ifdef PLUGIN_014
  ADDPLUGIN(014)
#endif

#ifdef PLUGIN_015
  ADDPLUGIN(015)
#endif

#ifdef PLUGIN_016
  ADDPLUGIN(016)
#endif

#ifdef PLUGIN_017
  ADDPLUGIN(017)
#endif

#ifdef PLUGIN_018
  ADDPLUGIN(018)
#endif

#ifdef PLUGIN_019
  ADDPLUGIN(019)
#endif

#ifdef PLUGIN_020
  ADDPLUGIN(020)
#endif

#ifdef PLUGIN_021
  ADDPLUGIN(021)
#endif

#ifdef PLUGIN_022
  ADDPLUGIN(022)
#endif

#ifdef PLUGIN_023
  ADDPLUGIN(023)
#endif

#ifdef PLUGIN_024
  ADDPLUGIN(024)
#endif

#ifdef PLUGIN_025
  ADDPLUGIN(025)
#endif

#ifdef PLUGIN_026
  ADDPLUGIN(026)
#endif

#ifdef PLUGIN_027
  ADDPLUGIN(027)
#endif

#ifdef PLUGIN_028
  ADDPLUGIN(028)
#endif

#ifdef PLUGIN_029
  ADDPLUGIN(029)
#endif

#ifdef PLUGIN_030
  ADDPLUGIN(030)
#endif

#ifdef PLUGIN_031
  ADDPLUGIN(031)
#endif

#ifdef PLUGIN_032
  ADDPLUGIN(032)
#endif

#ifdef PLUGIN_033
  ADDPLUGIN(033)
#endif

#ifdef PLUGIN_034
  ADDPLUGIN(034)
#endif

#ifdef PLUGIN_035
  ADDPLUGIN(035)
#endif

#ifdef PLUGIN_036
  ADDPLUGIN(036)
#endif

#ifdef PLUGIN_037
  ADDPLUGIN(037)
#endif

#ifdef PLUGIN_038
  ADDPLUGIN(038)
#endif

#ifdef PLUGIN_039
  ADDPLUGIN(039)
#endif

#ifdef PLUGIN_040
  ADDPLUGIN(040)
#endif

#ifdef PLUGIN_041
  ADDPLUGIN(041)
#endif

#ifdef PLUGIN_042
  ADDPLUGIN(042)
#endif

#ifdef PLUGIN_043
  ADDPLUGIN(043)
#endif

#ifdef PLUGIN_044
  ADDPLUGIN(044)
#endif

#ifdef PLUGIN_045
  ADDPLUGIN(045)
#endif

#ifdef PLUGIN_046
  ADDPLUGIN(046)
#endif

#ifdef PLUGIN_047
  ADDPLUGIN(047)
#endif

#ifdef PLUGIN_048
  ADDPLUGIN(048)
#endif

#ifdef PLUGIN_049
  ADDPLUGIN(049)
#endif

#ifdef PLUGIN_050
  ADDPLUGIN(050)
#endif

#ifdef PLUGIN_051
  ADDPLUGIN(051)
#endif

#ifdef PLUGIN_052
  ADDPLUGIN(052)
#endif

#ifdef PLUGIN_053
  ADDPLUGIN(053)
#endif

#ifdef PLUGIN_054
  ADDPLUGIN(054)
#endif

#ifdef PLUGIN_055
  ADDPLUGIN(055)
#endif

#ifdef PLUGIN_056
  ADDPLUGIN(056)
#endif

#ifdef PLUGIN_057
  ADDPLUGIN(057)
#endif

#ifdef PLUGIN_058
  ADDPLUGIN(058)
#endif

#ifdef PLUGIN_059
  ADDPLUGIN(059)
#endif

#ifdef PLUGIN_060
  ADDPLUGIN(060)
#endif

#ifdef PLUGIN_061
  ADDPLUGIN(061)
#endif

#ifdef PLUGIN_062
  ADDPLUGIN(062)
#endif

#ifdef PLUGIN_063
  ADDPLUGIN(063)
#endif

#ifdef PLUGIN_064
  ADDPLUGIN(064)
#endif

#ifdef PLUGIN_065
  ADDPLUGIN(065)
#endif

#ifdef PLUGIN_066
  ADDPLUGIN(066)
#endif

#ifdef PLUGIN_067
  ADDPLUGIN(067)
#endif

#ifdef PLUGIN_068
  ADDPLUGIN(068)
#endif

#ifdef PLUGIN_069
  ADDPLUGIN(069)
#endif

#ifdef PLUGIN_070
  ADDPLUGIN(070)
#endif

#ifdef PLUGIN_071
  ADDPLUGIN(071)
#endif

#ifdef PLUGIN_072
  ADDPLUGIN(072)
#endif

#ifdef PLUGIN_073
  ADDPLUGIN(073)
#endif

#ifdef PLUGIN_074
  ADDPLUGIN(074)
#endif

#ifdef PLUGIN_075
  ADDPLUGIN(075)
#endif

#ifdef PLUGIN_076
  ADDPLUGIN(076)
#endif

#ifdef PLUGIN_077
  ADDPLUGIN(077)
#endif

#ifdef PLUGIN_078
  ADDPLUGIN(078)
#endif

#ifdef PLUGIN_079
  ADDPLUGIN(079)
#endif

#ifdef PLUGIN_080
  ADDPLUGIN(080)
#endif

#ifdef PLUGIN_081
  ADDPLUGIN(081)
#endif

#ifdef PLUGIN_082
  ADDPLUGIN(082)
#endif

#ifdef PLUGIN_083
  ADDPLUGIN(083)
#endif

#ifdef PLUGIN_084
  ADDPLUGIN(084)
#endif

#ifdef PLUGIN_085
  ADDPLUGIN(085)
#endif

#ifdef PLUGIN_086
  ADDPLUGIN(086)
#endif

#ifdef PLUGIN_087
  ADDPLUGIN(087)
#endif

#ifdef PLUGIN_088
  ADDPLUGIN(088)
#endif

#ifdef PLUGIN_089
  ADDPLUGIN(089)
#endif

#ifdef PLUGIN_090
  ADDPLUGIN(090)
#endif

#ifdef PLUGIN_091
  ADDPLUGIN(091)
#endif

#ifdef PLUGIN_092
  ADDPLUGIN(092)
#endif

#ifdef PLUGIN_093
  ADDPLUGIN(093)
#endif

#ifdef PLUGIN_094
  ADDPLUGIN(094)
#endif

#ifdef PLUGIN_095
  ADDPLUGIN(095)
#endif

#ifdef PLUGIN_096
  ADDPLUGIN(096)
#endif

#ifdef PLUGIN_097
  ADDPLUGIN(097)
#endif

#ifdef PLUGIN_098
  ADDPLUGIN(098)
#endif

#ifdef PLUGIN_099
  ADDPLUGIN(099)
#endif

#ifdef PLUGIN_100
  ADDPLUGIN(100)
#endif

#ifdef PLUGIN_101
  ADDPLUGIN(101)
#endif

#ifdef PLUGIN_102
  ADDPLUGIN(102)
#endif

#ifdef PLUGIN_103
  ADDPLUGIN(103)
#endif

#ifdef PLUGIN_104
  ADDPLUGIN(104)
#endif

#ifdef PLUGIN_105
  ADDPLUGIN(105)
#endif

#ifdef PLUGIN_106
  ADDPLUGIN(106)
#endif

#ifdef PLUGIN_107
  ADDPLUGIN(107)
#endif

#ifdef PLUGIN_108
  ADDPLUGIN(108)
#endif

#ifdef PLUGIN_109
  ADDPLUGIN(109)
#endif

#ifdef PLUGIN_110
  ADDPLUGIN(110)
#endif

#ifdef PLUGIN_111
  ADDPLUGIN(111)
#endif

#ifdef PLUGIN_112
  ADDPLUGIN(112)
#endif

#ifdef PLUGIN_113
  ADDPLUGIN(113)
#endif

#ifdef PLUGIN_114
  ADDPLUGIN(114)
#endif

#ifdef PLUGIN_115
  ADDPLUGIN(115)
#endif

#ifdef PLUGIN_116
  ADDPLUGIN(116)
#endif

#ifdef PLUGIN_117
  ADDPLUGIN(117)
#endif

#ifdef PLUGIN_118
  ADDPLUGIN(118)
#endif

#ifdef PLUGIN_119
  ADDPLUGIN(119)
#endif

#ifdef PLUGIN_120
  ADDPLUGIN(120)
#endif

#ifdef PLUGIN_121
  ADDPLUGIN(121)
#endif

#ifdef PLUGIN_122
  ADDPLUGIN(122)
#endif

#ifdef PLUGIN_123
  ADDPLUGIN(123)
#endif

#ifdef PLUGIN_124
  ADDPLUGIN(124)
#endif

#ifdef PLUGIN_125
  ADDPLUGIN(125)
#endif

#ifdef PLUGIN_126
  ADDPLUGIN(126)
#endif

#ifdef PLUGIN_127
  ADDPLUGIN(127)
#endif

#ifdef PLUGIN_128
  ADDPLUGIN(128)
#endif

#ifdef PLUGIN_129
  ADDPLUGIN(129)
#endif

#ifdef PLUGIN_130
  ADDPLUGIN(130)
#endif

#ifdef PLUGIN_131
  ADDPLUGIN(131)
#endif

#ifdef PLUGIN_132
  ADDPLUGIN(132)
#endif

#ifdef PLUGIN_133
  ADDPLUGIN(133)
#endif

#ifdef PLUGIN_134
  ADDPLUGIN(134)
#endif

#ifdef PLUGIN_135
  ADDPLUGIN(135)
#endif

#ifdef PLUGIN_136
  ADDPLUGIN(136)
#endif

#ifdef PLUGIN_137
  ADDPLUGIN(137)
#endif

#ifdef PLUGIN_138
  ADDPLUGIN(138)
#endif

#ifdef PLUGIN_139
  ADDPLUGIN(139)
#endif

#ifdef PLUGIN_140
  ADDPLUGIN(140)
#endif

#ifdef PLUGIN_141
  ADDPLUGIN(141)
#endif

#ifdef PLUGIN_142
  ADDPLUGIN(142)
#endif

#ifdef PLUGIN_143
  ADDPLUGIN(143)
#endif

#ifdef PLUGIN_144
  ADDPLUGIN(144)
#endif

#ifdef PLUGIN_145
  ADDPLUGIN(145)
#endif

#ifdef PLUGIN_146
  ADDPLUGIN(146)
#endif

#ifdef PLUGIN_147
  ADDPLUGIN(147)
#endif

#ifdef PLUGIN_148
  ADDPLUGIN(148)
#endif

#ifdef PLUGIN_149
  ADDPLUGIN(149)
#endif

#ifdef PLUGIN_150
  ADDPLUGIN(150)
#endif

#ifdef PLUGIN_151
  ADDPLUGIN(151)
#endif

#ifdef PLUGIN_152
  ADDPLUGIN(152)
#endif

#ifdef PLUGIN_153
  ADDPLUGIN(153)
#endif

#ifdef PLUGIN_154
  ADDPLUGIN(154)
#endif

#ifdef PLUGIN_155
  ADDPLUGIN(155)
#endif

#ifdef PLUGIN_156
  ADDPLUGIN(156)
#endif

#ifdef PLUGIN_157
  ADDPLUGIN(157)
#endif

#ifdef PLUGIN_158
  ADDPLUGIN(158)
#endif

#ifdef PLUGIN_159
  ADDPLUGIN(159)
#endif

#ifdef PLUGIN_160
  ADDPLUGIN(160)
#endif

#ifdef PLUGIN_161
  ADDPLUGIN(161)
#endif

#ifdef PLUGIN_162
  ADDPLUGIN(162)
#endif

#ifdef PLUGIN_163
  ADDPLUGIN(163)
#endif

#ifdef PLUGIN_164
  ADDPLUGIN(164)
#endif

#ifdef PLUGIN_165
  ADDPLUGIN(165)
#endif

#ifdef PLUGIN_166
  ADDPLUGIN(166)
#endif

#ifdef PLUGIN_167
  ADDPLUGIN(167)
#endif

#ifdef PLUGIN_168
  ADDPLUGIN(168)
#endif

#ifdef PLUGIN_169
  ADDPLUGIN(169)
#endif

#ifdef PLUGIN_170
  ADDPLUGIN(170)
#endif

#ifdef PLUGIN_171
  ADDPLUGIN(171)
#endif

#ifdef PLUGIN_172
  ADDPLUGIN(172)
#endif

#ifdef PLUGIN_173
  ADDPLUGIN(173)
#endif

#ifdef PLUGIN_174
  ADDPLUGIN(174)
#endif

#ifdef PLUGIN_175
  ADDPLUGIN(175)
#endif

#ifdef PLUGIN_176
  ADDPLUGIN(176)
#endif

#ifdef PLUGIN_177
  ADDPLUGIN(177)
#endif

#ifdef PLUGIN_178
  ADDPLUGIN(178)
#endif

#ifdef PLUGIN_179
  ADDPLUGIN(179)
#endif

#ifdef PLUGIN_180
  ADDPLUGIN(180)
#endif

#ifdef PLUGIN_181
  ADDPLUGIN(181)
#endif

#ifdef PLUGIN_182
  ADDPLUGIN(182)
#endif

#ifdef PLUGIN_183
  ADDPLUGIN(183)
#endif

#ifdef PLUGIN_184
  ADDPLUGIN(184)
#endif

#ifdef PLUGIN_185
  ADDPLUGIN(185)
#endif

#ifdef PLUGIN_186
  ADDPLUGIN(186)
#endif

#ifdef PLUGIN_187
  ADDPLUGIN(187)
#endif

#ifdef PLUGIN_188
  ADDPLUGIN(188)
#endif

#ifdef PLUGIN_189
  ADDPLUGIN(189)
#endif

#ifdef PLUGIN_190
  ADDPLUGIN(190)
#endif

#ifdef PLUGIN_191
  ADDPLUGIN(191)
#endif

#ifdef PLUGIN_192
  ADDPLUGIN(192)
#endif

#ifdef PLUGIN_193
  ADDPLUGIN(193)
#endif

#ifdef PLUGIN_194
  ADDPLUGIN(194)
#endif

#ifdef PLUGIN_195
  ADDPLUGIN(195)
#endif

#ifdef PLUGIN_196
  ADDPLUGIN(196)
#endif

#ifdef PLUGIN_197
  ADDPLUGIN(197)
#endif

#ifdef PLUGIN_198
  ADDPLUGIN(198)
#endif

#ifdef PLUGIN_199
  ADDPLUGIN(199)
#endif

#ifdef PLUGIN_200
  ADDPLUGIN(200)
#endif

#ifdef PLUGIN_201
  ADDPLUGIN(201)
#endif

#ifdef PLUGIN_202
  ADDPLUGIN(202)
#endif

#ifdef PLUGIN_203
  ADDPLUGIN(203)
#endif

#ifdef PLUGIN_204
  ADDPLUGIN(204)
#endif

#ifdef PLUGIN_205
  ADDPLUGIN(205)
#endif

#ifdef PLUGIN_206
  ADDPLUGIN(206)
#endif

#ifdef PLUGIN_207
  ADDPLUGIN(207)
#endif

#ifdef PLUGIN_208
  ADDPLUGIN(208)
#endif

#ifdef PLUGIN_209
  ADDPLUGIN(209)
#endif

#ifdef PLUGIN_210
  ADDPLUGIN(210)
#endif

#ifdef PLUGIN_211
  ADDPLUGIN(211)
#endif

#ifdef PLUGIN_212
  ADDPLUGIN(212)
#endif

#ifdef PLUGIN_213
  ADDPLUGIN(213)
#endif

#ifdef PLUGIN_214
  ADDPLUGIN(214)
#endif

#ifdef PLUGIN_215
  ADDPLUGIN(215)
#endif

#ifdef PLUGIN_216
  ADDPLUGIN(216)
#endif

#ifdef PLUGIN_217
  ADDPLUGIN(217)
#endif

#ifdef PLUGIN_218
  ADDPLUGIN(218)
#endif

#ifdef PLUGIN_219
  ADDPLUGIN(219)
#endif

#ifdef PLUGIN_220
  ADDPLUGIN(220)
#endif

#ifdef PLUGIN_221
  ADDPLUGIN(221)
#endif

#ifdef PLUGIN_222
  ADDPLUGIN(222)
#endif

#ifdef PLUGIN_223
  ADDPLUGIN(223)
#endif

#ifdef PLUGIN_224
  ADDPLUGIN(224)
#endif

#ifdef PLUGIN_225
  ADDPLUGIN(225)
#endif

#ifdef PLUGIN_226
  ADDPLUGIN(226)
#endif

#ifdef PLUGIN_227
  ADDPLUGIN(227)
#endif

#ifdef PLUGIN_228
  ADDPLUGIN(228)
#endif

#ifdef PLUGIN_229
  ADDPLUGIN(229)
#endif

#ifdef PLUGIN_230
  ADDPLUGIN(230)
#endif

#ifdef PLUGIN_231
  ADDPLUGIN(231)
#endif

#ifdef PLUGIN_232
  ADDPLUGIN(232)
#endif

#ifdef PLUGIN_233
  ADDPLUGIN(233)
#endif

#ifdef PLUGIN_234
  ADDPLUGIN(234)
#endif

#ifdef PLUGIN_235
  ADDPLUGIN(235)
#endif

#ifdef PLUGIN_236
  ADDPLUGIN(236)
#endif

#ifdef PLUGIN_237
  ADDPLUGIN(237)
#endif

#ifdef PLUGIN_238
  ADDPLUGIN(238)
#endif

#ifdef PLUGIN_239
  ADDPLUGIN(239)
#endif

#ifdef PLUGIN_240
  ADDPLUGIN(240)
#endif

#ifdef PLUGIN_241
  ADDPLUGIN(241)
#endif

#ifdef PLUGIN_242
  ADDPLUGIN(242)
#endif

#ifdef PLUGIN_243
  ADDPLUGIN(243)
#endif

#ifdef PLUGIN_244
  ADDPLUGIN(244)
#endif

#ifdef PLUGIN_245
  ADDPLUGIN(245)
#endif

#ifdef PLUGIN_246
  ADDPLUGIN(246)
#endif

#ifdef PLUGIN_247
  ADDPLUGIN(247)
#endif

#ifdef PLUGIN_248
  ADDPLUGIN(248)
#endif

#ifdef PLUGIN_249
  ADDPLUGIN(249)
#endif

#ifdef PLUGIN_250
  ADDPLUGIN(250)
#endif

#ifdef PLUGIN_251
  ADDPLUGIN(251)
#endif

#ifdef PLUGIN_252
  ADDPLUGIN(252)
#endif

#ifdef PLUGIN_253
  ADDPLUGIN(253)
#endif

#ifdef PLUGIN_254
  ADDPLUGIN(254)
#endif

#ifdef PLUGIN_255
  ADDPLUGIN(255)
#endif

#ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ADDPLUGIN(...)"));
#endif

  String dummy;
  PluginCall(PLUGIN_DEVICE_ADD, nullptr, dummy);
    // Set all not supported plugins to disabled.
  for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
    if (!supportedPluginID(Settings.TaskDeviceNumber[taskIndex])) {
      Settings.TaskDeviceEnabled[taskIndex] = false;
    }
  }

#ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PLUGIN_DEVICE_ADD"));
#endif

  PluginCall(PLUGIN_INIT_ALL, nullptr, dummy);
#ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PLUGIN_INIT_ALL"));
#endif

  sortDeviceIndexArray(); // Used in device selector dropdown.
}

