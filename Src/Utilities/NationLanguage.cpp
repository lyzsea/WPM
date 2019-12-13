#include "NationLanguage.h"
#include <windows.h>
#include <WinNls.h>


#define countof(x) (sizeof(x) / sizeof(x[0]))
const int lcid = ::GetSystemDefaultLangID();
#define MAX_LANG_SYSNAME_LEN    20

enum ELangAreaList
{
	LA_UNKNOWN				= -1,
	LA_OTHER                = 0,
	LA_AF_ZA				= 0x0436,		//0x0436 Afrikaans (South Africa) af-ZA Latn 1252
	LA_SQ_AL				= 0x041c,		//0x041c Albanian (Albania) sq-AL Latn 1252 
	LA_GSW_FR			= 0x0484,		//0x0484 Windows Vista and later: Alsatian (France) gsw-FR     
	LA_AM_ET				= 0x045e,		//0x045e Windows Vista and later: Amharic (Ethiopia) am-ET   Unicode only 
	LA_AR_DZ				= 0x1401,		//0x1401 Arabic (Algeria) ar-DZ Arab 1256 
	LA_AR_BH				= 0x3c01,		//0x3c01 Arabic (Bahrain) ar-BH Arab 1256 
	LA_AR_EG				= 0x0c01,		//0x0c01 Arabic (Egypt) ar-EG Arab 1256 
	LA_AR_IQ				= 0x0801,		//0x0801 Arabic (Iraq) ar-IQ Arab 1256 
	LA_AR_JO				= 0x2c01,		//0x2c01 Arabic (Jordan) ar-JO Arab 1256 
	LA_AR_KW			= 0x3401,		//0x3401 Arabic (Kuwait) ar-KW Arab 1256 
	LA_AR_LB				= 0x3001,		//0x3001 Arabic (Lebanon) ar-LB Arab 1256 
	LA_AR_LY				= 0x1001,		//0x1001 Arabic (Libya) ar-LY Arab 1256 
	LA_AR_MA			= 0x1801,		//0x1801 Arabic (Morocco) ar-MA Arab 1256 
	LA_AR_OM			= 0x2001,		//0x2001 Arabic (Oman) ar-OM Arab 1256 
	LA_AR_QA				= 0x4001,		//0x4001 Arabic (Qatar) ar-QA Arab 1256 
	LA_AR_SA				= 0x0401,		//0x0401 Arabic (Saudi Arabia) ar-SA Arab 1256 
	LA_AR_SY				= 0x2801,		//0x2801 Arabic (Syria) ar-SY Arab 1256 
	LA_AR_TN				= 0x1c01,		//0x1c01 Arabic (Tunisia) ar-TN Arab 1256 
	LA_AR_AE				= 0x3801,		//0x3801 Arabic (U.A.E.) ar-AE Arab 1256 
	LA_AR_YE				= 0x2401,		//0x2401 Arabic (Yemen) ar-YE Arab 1256 
	LA_HY_AM			= 0x042b,		//0x042b Windows 2000 and later: Armenian (Armenia) hy-AM Armn Unicode only 
	LA_AS_IN				= 0x044d,		//0x044d Windows Vista and later: Assamese (India) as-IN   Unicode only 
	LA_AZ_CYRL_AZ	= 0x082c,		//0x082c Azeri (Azerbaijan,		Cyrillic) az-Cyrl-AZ Cyrl 1251 
	LA_AZ_LATN_AZ	= 0x042c,		//0x042c Azeri (Azerbaijan,		Latin) az-Latn-AZ Latn 1254 
	LA_BA_RU				= 0x046d,		//0x046d Windows Vista and later: Bashkir (Russia) ba-RU     
	LA_EU_ES				= 0x042d,		//0x042d Basque (Basque) eu-ES Latn 1252 
	LA_BE_BY				= 0x0423,		//0x0423 Belarusian (Belarus) be-BY Cyrl 1251 
	LA_BN_IN				= 0x0445,		//0x0445 Windows XP SP2 and later: Bengali (India) bn-IN Beng Unicode only 
	LA_BS_CYRL_BA	= 0x201a,		//0x201a Windows XP SP2 and later (downloadable); Windows Vista and later: Bosnian (Bosnia and Herzegovina,		Cyrillic) bs-Cyrl-BA Cyrl 1251 
	LA_BS_LATN_BA	= 0x141a,		//0x141a Windows XP SP2 and later: Bosnian (Bosnia and Herzegovina,		Latin) bs-Latn-BA Latn 1250 
	LA_BR_FR				= 0x047e,		//0x047e Breton (France) br-FR Latn 1252 
	LA_BG_BG			= 0x0402,		//0x0402 Bulgarian (Bulgaria) bg-BG Cyrl 1251 
	LA_CA_ES				= 0x0403,		//0x0403 Catalan (Catalan) ca-ES Latn 1252 
	LA_ZH_HK			= 0x0c04,		//0x0c04 Chinese (Hong Kong SAR,		PRC) zh-HK Hant 950 
	LA_ZH_MO			= 0x1404,		//0x1404 Windows 98/Me,		Windows XP and later: Chinese (Macao SAR) zh-MO Hant 950 
	LA_ZH_CN			= 0x0804,		//0x0804 Chinese (PRC) zh-CN Hans 936 
	LA_ZH_SG			= 0x1004,		//0x1004 Chinese (Singapore) zh-SG Hans 936 
	LA_ZH_TW			= 0x0404,		//0x0404 Chinese (Taiwan) zh-TW Hant 950 
	LA_HR_BA				= 0x101a,		//0x101a Windows XP SP2 and later: Croatian (Bosnia and Herzegovina,		Latin) hr-BA Latn 1250 
	LA_HR_HR			= 0x041a,		//0x041a Croatian (Croatia) hr-HR Latn 1250 
	LA_CS_CZ				= 0x0405,		//0x0405 Czech (Czech Republic) cs-CZ Latn 1250 
	LA_DA_DK			= 0x0406,		//0x0406 Danish (Denmark) da-DK Latn 1252 
	LA_GBZ_AF			= 0x048c,		//0x048c Windows XP and later: Dari (Afghanistan) gbz-AF Arab 1256 
	LA_DV_MV			= 0x0465,		//0x0465 Windows XP and later: Divehi (Maldives) dv-MV Thaa Unicode only 
	LA_NL_BE				= 0x0813,		//0x0813 Dutch (Belgium) nl-BE Latn 1252 
	LA_NL_NL				= 0x0413,		//0x0413 Dutch (Netherlands) nl-NL Latn 1252 
	LA_EN_AU				= 0x0c09,		//0x0c09 English (Australia) en-AU Latn 1252 
	LA_EN_BZ				= 0x2809,		//0x2809 English (Belize) en-BZ Latn 1252 
	LA_EN_CA				= 0x1009,		//0x1009 English (Canada) en-CA Latn 1252 
	LA_EN_029			= 0x2409,//0x2409 English (Caribbean) en-029 Latn 1252 
	LA_EN_IE				= 0x4009,		//0x4009 Windows Vista and later: English (India) en-IN Latn 1252 
	LA_EN_IN				= 0x1809,		//0x1809 English (Ireland) en-IE Latn 1252 
	LA_EN_JM				= 0x2009,		//0x2009 English (Jamaica) en-JM Latn 1252 
	LA_EN_MY				= 0x4409,		//0x4409 Windows Vista and later: English (Malaysia) en-MY Latn 1252 
	LA_EN_NZ				= 0x1409,		//0x1409 English (New Zealand) en-NZ Latn 1252 
	LA_EN_PH				= 0x3409,		//0x3409 Windows 98/Me,		Windows 2000 and later: English (Philippines) en-PH Latn 1252 
	LA_EN_SG				= 0x4809,		//0x4809 Windows Vista and later: English (Singapore) en-SG Latn 1252 
	LA_EN_ZA				= 0x1c09,		//0x1c09 English (South Africa) en-ZA Latn 1252 
	LA_EN_TT				= 0x2c09,		//0x2c09 English (Trinidad and Tobago) en-TT Latn 1252 
	LA_EN_GB				= 0x0809,		//0x0809 English (United Kingdom) en-GB Latn 1252 
	LA_EN_US				= 0x0409,		//0x0409 English (United States) en-US Latn 1252 
	LA_EN_ZW			= 0x3009,		//0x3009 Windows 98/Me,		Windows 2000 and later: English (Zimbabwe) en-ZW Latn 1252 
	LA_ET_EE				= 0x0425,		//0x0425 Estonian (Estonia) et-EE Latn 1257 
	LA_FO_FO				= 0x0438,		//0x0438 Faroese (Faroe Islands) fo-FO Latn 1252 
	LA_FIL_PH			= 0x0464,		//0x0464 Windows XP SP2 and later (downloadable); Windows Vista and later: Filipino (Philippines) fil-PH Latn 1252 
	LA_FI_FI				= 0x040b,		//0x040b Finnish (Finland) fi-FI Latn 1252 
	LA_FR_BE				= 0x080c,		//0x080c French (Belgium) fr-BE Latn 1252 
	LA_FR_CA				= 0x0c0c,		//0x0c0c French (Canada) fr-CA Latn 1252 
	LA_FR_FR				= 0x040c,		//0x040c French (France) fr-FR Latn 1252 
	LA_FR_LU				= 0x140c,		//0x140c French (Luxembourg) fr-LU Latn 1252 
	LA_FR_MC				= 0x180c,		//0x180c French (Monaco) fr-MC Latn 1252 
	LA_FR_CH				= 0x100c,		//0x100c French (Switzerland) fr-CH Latn 1252 
	LA_FY_NL				= 0x0462,		//0x0462 Windows XP SP2 and later (downloadable); Windows Vista and later: Frisian (Netherlands) fy-NL Latn 1252 
	LA_MM_MM				= 0x0455,
	LA_GL_ES				= 0x0456,		//0x0456 Windows XP and later: Galician (Spain) gl-ES Latn 1252 
	LA_KA_GE				= 0x0437,		//0x0437 Windows 2000 and later: Georgian (Georgia) ka-GE Geor Unicode only 
	LA_DE_AT				= 0x0c07,		//0x0c07 German (Austria) de-AT Latn 1252 
	LA_DE_DE				= 0x0407,		//0x0407 German (Germany) de-DE Latn 1252 
	LA_DE_LI				= 0x1407,		//0x1407 German (Liechtenstein) de-LI Latn 1252 
	LA_DE_LU				= 0x1007,		//0x1007 German (Luxembourg) de-LU Latn 1252 
	LA_DE_CH			= 0x0807,		//0x0807 German (Switzerland) de-CH Latn 1252 
	LA_EL_GR				= 0x0408,		//0x0408 Greek (Greece) el-GR Grek 1253 
	LA_KL_GL				= 0x046f,		//0x046f Windows Vista and later: Greenlandic (Greenland) kl-GL Latn 1252 
	LA_GU_IN				= 0x0447,		//0x0447 Windows XP and later: Gujarati (India) gu-IN Gujr Unicode only 
	LA_HA_LATN_NG	= 0x0468,		//0x0468 Windows Vista and later: Hausa (Nigeria,		Latin) ha-Latn-NG Latn 1252 
	LA_HE_IL				= 0x040d,		//0x040d Hebrew (Israel) he-IL Hebr 1255 
	LA_HI_IN				= 0x0439,		//0x0439 Windows 2000 and later: Hindi (India) hi-IN Deva Unicode only 
	LA_HU_HU			= 0x040e,		//0x040e Hungarian (Hungary) hu-HU Latn 1250 
	LA_IS_IS				= 0x040f,		//0x040f Icelandic (Iceland) is-IS Latn 1252 
	LA_IG_NG				= 0x0470,		//0x0470 Igbo (Nigeria) ig-NG     
	LA_ID_ID				= 0x0421,		//0x0421 Indonesian (Indonesia) id-ID Latn 1252 
	LA_IU_LATN_CA	= 0x085d,		//0x085d Windows XP and later: Inuktitut (Canada,		Latin) iu-Latn-CA Latn 1252 
	LA_IU_CANS_CA	= 0x045d,		//0x045d Windows XP SP2 and later (downloadable); Windows Vista and later: Inuktitut (Canada,		Syllabics) iu-Cans-CA Cans Unicode only 
	LA_GA_IE				= 0x083c,		//0x083c Windows XP SP2 and later (downloadable); Windows Vista and later: Irish (Ireland) ga-IE Latn 1252 
	LA_IT_IT				= 0x0410,		//0x0410 Italian (Italy) it-IT Latn 1252 
	LA_IT_CH				= 0x0810,		//0x0810 Italian (Switzerland) it-CH Latn 1252 
	LA_JA_JP				= 0x0411,		//0x0411 Japanese (Japan) ja-JP Hani;Hira;Kana 932 
	LA_KN_IN				= 0x044b,		//0x044b Windows XP and later: Kannada (India) kn-IN Knda Unicode only 
	LA_KK_KZ				= 0x043f ,		//0x043f Windows 2000 and later: Kazakh (Kazakhstan) kk-KZ Cyrl 1251 
	LA_KH_KH			= 0x0453,		//0x0453 Windows Vista and later: Khmer (Cambodia) kh-KH Khmr Unicode only 
	LA_GUT_GT			= 0x0486,		//0x0486 Windows Vista and later: K'iche (Guatemala) qut-GT Latn 1252 
	LA_RW_RW			= 0x0487,		//0x0487 Windows Vista and later: Kinyarwanda (Rwanda) rw-RW Latn 1252 
	LA_KOK_IN			= 0x0457,		//0x0457 Windows 2000 and later: Konkani (India) kok-IN Deva Unicode only 
	//    LA_KOREAN				= 0x0812,		//0x0812 Windows 95,		Windows NT 4.0 only: Korean (Johab)       
	LA_KO_KR				= 0x0412,		//0x0412 Korean (Korea) ko-KR Hang;Hani 949 
	LA_KY_KG				= 0x0440,		//0x0440 Windows XP and later: Kyrgyz (Kyrgyzstan) ky-KG Cyrl 1251 
	LA_LO_LA				= 0x0454,		//0x0454 Windows Vista and later: Lao (Lao PDR) lo-LA Laoo Unicode only 
	LA_LV_LV				= 0x0426,		//0x0426 Latvian (Latvia) lv-LV Latn 1257 
	LA_IT_T				= 0x0427,		//0x0427 Lithuanian (Lithuania) lt-LT Latn 1257 
	LA_DSB_DE			= 0x082e,		//0x082e Windows Vista and later: Lower Sorbian (Germany) dsb-DE Latn 1252 
	LA_LB_LU				= 0x046e,		//0x046e Windows XP SP2 and later (downloadable); Windows Vista and later: Luxembourgish (Luxembourg) lb-LU Latn 1252 
	LA_MK_MK			= 0x042f,		//0x042f Windows 2000 and later: Macedonian (Macedonia,		FYROM) mk-MK Cyrl 1251 
	LA_MS_BN			= 0x083e,		//0x083e Windows 2000 and later: Malay (Brunei Darussalam) ms-BN Latn 1252 
	LA_MS_MY			= 0x043e,		//0x043e Windows 2000 and later: Malay (Malaysia) ms-MY Latn 1252 
	LA_ML_IN				= 0x044c,		//0x044c Windows XP SP2 and later: Malayalam (India) ml-IN Mlym Unicode only 
	LA_MT_MT			= 0x043a,		//0x043a Windows XP SP2 and later: Maltese (Malta) mt-MT Latn 1252 
	LA_MI_NZ				= 0x0481,		//0x0481 Windows XP SP2 and later: Maori (New Zealand) mi-NZ Latn 1252 
	LA_ARN_CL			= 0x047a,		//0x047a Windows XP SP2 and later (downloadable); Windows Vista and later: Mapudungun (Chile) arn-CL Latn 1252 
	LA_MR_IN				= 0x044e,		//0x044e Windows 2000 and later: Marathi (India) mr-IN Deva Unicode only 
	LA_MOH_CA			= 0x047c,		//0x047c Windows XP SP2 and later (downloadable); Windows Vista and later: Mohawk (Canada) moh-CA Latn 1252 
	LA_MN_CYRL_MN	= 0x0450,		//0x0450 Windows XP and later: Mongolian (Mongolia) mn-Cyrl-MN Cyrl 1251 
	LA_MN_MONG_CN	= 0x0850,		//0x0850 Windows Vista and later: Mongolian (PRC) mn-Mong-CN Mong Unicode only 
	LA_NE_NP				= 0x0461,		//0x0461 Windows XP SP2 and later (downloadable); Windows Vista and later: Nepali (Nepal) ne-NP Deva Unicode only 
	LA_NB_NO			= 0x0414,		//0x0414 Norwegian (Bokmål,		Norway) nb-NO Latn 1252 
	LA_NN_NO			= 0x0814,		//0x0814 Norwegian (Nynorsk,		Norway) nn-NO Latn 1252 
	LA_OC_FR				= 0x0482,		//0x0482 Occitan (France) oc-FR Latn 1252 
	LA_OR_IN				= 0x0448,		//0x0448 Oriya (India) or-IN Orya Unicode only 
	LA_PS_AF				= 0x0463,		                            //0x0463 Windows XP SP2 and later (downloadable); Windows Vista and later: Pashto (Afghanistan) ps-AF     
	LA_FA_IR				= 0x0429,		//0x0429 Persian (Iran) fa-IR Arab 1256 
	LA_PL_PL				= 0x0415,		//0x0415 Polish (Poland) pl-PL Latn 1250 
	LA_PT_BR				= 0x0416,		//0x0416 Portuguese (Brazil) pt-BR Latn 1252 
	LA_PT_PT				= 0x0816,		//0x0816 Portuguese (Portugal) pt-PT Latn 1252 
	LA_PA_IN				= 0x0446,		//0x0446 Windows XP and later: Punjabi (India) pa-IN Guru Unicode only 
	LA_QUZ_BO			= 0x046b,		//0x046b Windows XP SP2 and later: Quechua (Bolivia) quz-BO Latn 1252 
	LA_QUZ_EC			= 0x086b,		//0x086b Windows XP SP2 and later: Quechua (Ecuador) quz-EC Latn 1252 
	LA_QUZ_PE			= 0x0c6b,		//0x0c6b Windows XP SP2 and later: Quechua (Peru) quz-PE Latn 1252 
	LA_RO_RO			= 0x0418,		//0x0418 Romanian (Romania) ro-RO Latn 1250 
	LA_RM_CH			= 0x0417,		//0x0417 Windows XP SP2 and later (downloadable); Windows Vista and later: Romansh (Switzerland) rm-CH Latn 1252 
	LA_RU_RU			= 0x0419,		//0x0419 Russian (Russia) ru-RU Cyrl 1251 
	LA_SMN_FI			= 0x243b,		//0x243b Windows XP SP2 and later: Sami (Inari,		Finland) smn-FI Latn 1252 
	LA_SMJ_NO			= 0x103b,		//0x103b Windows XP SP2 and later: Sami (Lule,		Norway) smj-NO Latn 1252 
	LA_SMJ_SE			= 0x143b,		//0x143b Windows XP SP2 and later: Sami (Lule,		Sweden) smj-SE Latn 1252 
	LA_SE_FI				= 0x0c3b,		//0x0c3b Windows XP SP2 and later: Sami (Northern,		Finland) se-FI Latn 1252 
	LA_SE_NO				= 0x043b,		//0x043b Windows XP SP2 and later: Sami (Northern,		Norway) se-NO Latn 1252 
	LA_SE_SE				= 0x083b,		//0x083b Windows XP SP2 and later: Sami (Northern,		Sweden) se-SE Latn 1252 
	LA_SMS_FI			= 0x203b,		//0x203b Windows XP SP2 and later: Sami (Skolt,		Finland) sms-FI Latn 1252 
	LA_SMA_NO			= 0x183b,		//0x183b Windows XP SP2 and later: Sami (Southern,		Norway) sma-NO Latn 1252 
	LA_SMA_SE			= 0x1c3b,		//0x1c3b Windows XP SP2 and later: Sami (Southern,		Sweden) sma-SE Latn 1252 
	LA_SA_IN				= 0x044f,		//0x044f Windows 2000 and later: Sanskrit (India) sa-IN Deva Unicode only 
	LA_SR_CYRL_BA	= 0x1c1a,		//0x1c1a Windows XP SP2 and later: Serbian (Bosnia and Herzegovina,		Cyrillic) sr-Cyrl-BA Cyrl 1251 
	LA_SR_LATN_BA	= 0x181a,		//0x181a Windows XP SP2 and later: Serbian (Bosnia and Herzegovina,		Latin) sr-Latn-BA Latn 1250 
	LA_SR_CYRL_CS	= 0x0c1a,		//0x0c1a Serbian (Serbia,		Cyrillic) sr-Cyrl-CS Cyrl 1251 
	LA_SR_LATN_CS	= 0x081a,		//0x081a Serbian (Serbia,		Latin) sr-Latn-CS Latn 1250 
	LA_NS_ZA				= 0x046c,		//0x046c Windows XP SP2 and later: Sesotho sa Leboa/Northern Sotho (South Africa) ns-ZA Latn 1252 
	LA_TN_ZA				= 0x0432,		//0x0432 Windows XP SP2 and later: Setswana/Tswana (South Africa) tn-ZA Latn 1252 
	LA_SI_LK				= 0x045b,		//0x045b Windows Vista and later: Sinhala (Sri Lanka) si-LK Sinh Unicode only 
	LA_SK_SK				= 0x041b,		//0x041b Slovak (Slovakia) sk-SK Latn 1250 
	LA_SL_SI				= 0x0424,		//0x0424 Slovenian (Slovenia) sl-SI Latn 1250 
	LA_ES_AR				= 0x2c0a,		//0x2c0a Spanish (Argentina) es-AR Latn 1252 
	LA_ES_BO				= 0x400a,		//0x400a Spanish (Bolivia) es-BO Latn 1252 
	LA_ES_CL				= 0x340a,		//0x340a Spanish (Chile) es-CL Latn 1252 
	LA_ES_CO				= 0x240a,		//0x240a Spanish (Colombia) es-CO Latn 1252 
	LA_ES_CR				= 0x140a,		//0x140a Spanish (Costa Rica) es-CR Latn 1252 
	LA_ES_DO				= 0x1c0a,		//0x1c0a Spanish (Dominican Republic) es-DO Latn 1252 
	LA_ES_EC				= 0x300a,		//0x300a Spanish (Ecuador) es-EC Latn 1252 
	LA_ES_SV				= 0x440a,		//0x440a Spanish (El Salvador) es-SV Latn 1252 
	LA_ES_GT				= 0x100a,		//0x100a Spanish (Guatemala) es-GT Latn 1252 
	LA_ES_HN				= 0x480a,		//0x480a Spanish (Honduras) es-HN Latn 1252 
	LA_ES_MX				= 0x080a,		//0x080a Spanish (Mexico) es-MX Latn 1252 
	LA_ES_NI				= 0x4c0a,		//0x4c0a Spanish (Nicaragua) es-NI Latn 1252 
	LA_ES_PA				= 0x180a,		//0x180a Spanish (Panama) es-PA Latn 1252 
	LA_ES_PY				= 0x3c0a,		//0x3c0a Spanish (Paraguay) es-PY Latn 1252 
	LA_ES_PE				= 0x280a,		//0x280a Spanish (Peru) es-PE Latn 1252 
	LA_ES_PR				= 0x500a,		//0x500a Spanish (Puerto Rico) es-PR Latn 1252 
	LA_ES_ES				= 0x0c0a,		//0x0c0a Spanish (Spain) es-ES Latn 1252 
	LA_ES_ES_TRADNL	= 0x040a,		//0x040a Spanish (Spain,		Traditional Sort) es-ES_tradnl Latn 1252 
	LA_ES_US				= 0x540a,		//0x540a Windows Vista and later: Spanish (United States) es-US     
	LA_ES_UY				= 0x380a,		//0x380a Spanish (Uruguay) es-UY Latn 1252 
	LA_ES_VE				= 0x200a,		//0x200a Spanish (Venezuela) es-VE Latn 1252 
	LA_SW_KE			= 0x0441,		//0x0441 Windows 2000 and later: Swahili (Kenya) sw-KE Latn 1252 
	LA_SV_FI				= 0x081d,		//0x081d Swedish (Finland) sv-FI Latn 1252 
	LA_SV_SE				= 0x041d,		//0x041d Swedish (Sweden) sv-SE Latn 1252 
	LA_SYR_SY			= 0x045a,		//0x045a Windows XP and later: Syriac (Syria) syr-SY Syrc Unicode only 
	LA_TG_CYRL_TJ	= 0x0428,		//0x0428 Windows Vista and later: Tajik (Tajikistan) tg-Cyrl-TJ Cyrl 1251 
	LA_TMZ_LATN_DZ	= 0x085f,		//0x085f Windows Vista and later: Tamazight (Algeria,		Latin) tmz-Latn-DZ Latn 1252 
	LA_TA_IN				= 0x0449,		//0x0449 Windows 2000 and later: Tamil (India) ta-IN Taml Unicode only 
	LA_TT_RU				= 0x0444,		//0x0444 Windows XP and later: Tatar (Russia) tt-RU Cyrl 1251 
	LA_TE_IN				= 0x044a,		//0x044a Windows XP and later: Telugu (India) te-IN Telu Unicode only 
	LA_TH_TH				= 0x041e,		//0x041e Thai (Thailand) th-TH Thai 874 
	LA_BO_BT				= 0x0851,		//0x0851 Windows Vista and later: Tibetan (Bhutan) bo-BT Tibt Unicode only 
	LA_BO_CN			= 0x0451,		//0x0451 Windows Vista and later: Tibetan (PRC) bo-CN Tibt Unicode only 
	LA_TR_TR				= 0x041f,		//0x041f Turkish (Turkey) tr-TR Latn 1254 
	LA_TK_TM				= 0x0442,		//0x0442 Windows Vista and later: Turkmen (Turkmenistan) tk-TM Cyrl 1251 
	LA_UG_CN			= 0x0480,		//0x0480 Windows Vista and later: Uighur (PRC) ug-CN Arab 1256 
	LA_UK_UA				= 0x0422,		//0x0422 Ukrainian (Ukraine) uk-UA Cyrl 1251 
	LA_WEN_DE			= 0x042e,		//0x042e Windows Vista and later: Upper Sorbian (Germany) wen-DE Latn 1252 
	LA_TR_IN				= 0x0820,		//0x0820 Urdu (India) tr-IN     
	LA_UR_PK				= 0x0420,		//0x0420 Windows 98/Me,		Windows 2000 and later: Urdu (Pakistan) ur-PK Arab 1256 
	LA_UZ_CYRL_UZ	= 0x0843,		//0x0843 Windows 2000 and later: Uzbek (Uzbekistan,		Cyrillic) uz-Cyrl-UZ Cyrl 1251 
	LA_UZ_LATN_UZ	= 0x0443,		//0x0443 Windows 2000 and later: Uzbek (Uzbekistan,		Latin) uz-Latn-UZ Latn 1254 
	LA_VI_VN				= 0x042a,		//0x042a Windows 98/Me,		Windows NT 4.0 and later: Vietnamese (Vietnam) vi-VN Latn 1258 
	LA_CY_GB				= 0x0452,		//0x0452 Windows XP SP2 and later: Welsh (United Kingdom) cy-GB Latn 1252 
	LA_WO_SN			= 0x0488,		//0x0488 Windows Vista and later: Wolof (Senegal) wo-SN Latn 1252 
	LA_XH_ZA				= 0x0434,		//0x0434 Windows XP SP2 and later: Xhosa/isiXhosa (South Africa) xh-ZA Latn 1252 
	LA_SAH_RU			= 0x0485,		//0x0485 Windows Vista and later: Yakut (Russia) sah-RU Cyrl 1251 
	LA_II_CN				= 0x0478,		//0x0478 Windows Vista and later: Yi (PRC) ii-CN Yiii Unicode only 
	LA_YO_NG			= 0x046a,		//0x046a Windows Vista and later: Yoruba (Nigeria) yo-NG     
	LA_ZU_ZA				= 0x0435,		//0x0435 Windows XP SP2 and later: Zulu/isiZulu (South Africa) zu-ZA Latn 1252 
	LA_MAX				= 0xFFFF,
};
struct tagAreaItem
{
	ELangAreaList	  langID;
	wchar_t           langNationName[MAX_LANG_SYSNAME_LEN];
	wchar_t           langLanName[MAX_LANG_SYSNAME_LEN]; 
};
tagAreaItem downloader_lang[] = 
{   
	{LA_PT_BR,  			L"br",      L"pt"},
	{LA_TR_TR,				L"tr",      L"tr"},
	{LA_PL_PL,				L"pl",      L"pl"},
	{LA_EN_US,				L"us",		L"en"},
	{LA_VI_VN,				L"vn",      L"vi"},
	{LA_TH_TH,  		    L"th",      L"th"},
	{LA_ES_MX,				L"mx",      L"es"},
	{LA_ES_ES,				L"es",      L"es"},
	{LA_ES_AR,  			L"ar",      L"es"},
	{LA_ZH_CN,				L"cn",      L"zh"},
	{LA_SV_SE,				L"sw",      L"sv"},
	{LA_KO_KR,				L"kr",      L"ko"},
	{LA_AR_AE,				L"ae",      L"ar"},
	{LA_JA_JP,				L"jp",      L"ja"},
	{LA_NL_NL,				L"nl",      L"nl"},
	{LA_HI_IN,				L"in",      L"hi"},
	{LA_HU_HU,				L"hu",      L"hu"},
	{LA_ZH_TW,  		    L"tw",      L"zh"},
	{LA_DE_DE,				L"de",      L"de"},
	{LA_DE_AT,				L"at",      L"de"},
	{LA_EN_AU,				L"au",      L"en"},
	{LA_IT_IT,				L"it",      L"it"},
	{LA_AR_EG,				L"eg",      L"ar"},
	{LA_RO_RO,				L"ro",      L"ro"},
	{LA_AR_SA,				L"sa",      L"ar"},
	{LA_DA_DK,				L"dk",      L"da"},
	{LA_AR_MA,				L"ma",      L"ar"},
	{LA_EN_MY,				L"my",      L"en"},
	{LA_MM_MM,				L"mm",      L"mm"},
	{LA_PT_PT,				L"pt",      L"pt"},
	{LA_FR_FR,				L"fr",      L"fr"},
};

namespace NationLanguage
{

	std::wstring GetLangName(std::wstring& strLangName)
	{
		strLangName = L"en";
		for(int i=0; i< countof(downloader_lang); i++)
		{
			if((int)(downloader_lang[i].langID) == lcid)
			{
				strLangName = downloader_lang[i].langLanName;
			}
		}

		return strLangName;
	}

	std::wstring GetNationName(std::wstring& strNationName)
	{
		strNationName = L"en";
		for(int i=0; i< countof(downloader_lang); i++)
		{
			if((int)(downloader_lang[i].langID) == lcid)
			{
				strNationName = downloader_lang[i].langNationName;
			}
		}

		return strNationName;
	}
}



