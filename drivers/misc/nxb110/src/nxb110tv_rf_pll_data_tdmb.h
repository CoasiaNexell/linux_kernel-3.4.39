/******************************************************************************** 
* (c) COPYRIGHT 2010 NEXELL, Inc. ALL RIGHTS RESERVED.
* 
* This software is the property of NEXELL and is furnished under license by NEXELL.                
* This software may be used only in accordance with the terms of said license.                         
* This copyright noitce may not be remoced, modified or obliterated without the prior                  
* written permission of NEXELL, Inc.                                                                 
*                                                                                                      
* This software may not be copied, transmitted, provided to or otherwise made available                
* to any other person, company, corporation or other entity except as specified in the                 
* terms of said license.                                                                               
*                                                                                                      
* No right, title, ownership or other interest in the software is hereby granted or transferred.       
*                                                                                                      
* The information contained herein is subject to change without notice and should 
* not be construed as a commitment by NEXELL, Inc.                                                                    
* 
* TITLE 	  : NEXELL TV RF PLL data for TDMB. 
*
* FILENAME    : nxb110tv_rf_pll_data_tdmb.h
*
* DESCRIPTION : 
*		All the declarations and definitions necessary for the NEXELL TV driver.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 09/30/2010  Ko, Kevin        Created the file.
********************************************************************************/


#if (RTV_SRC_CLK_FREQ_KHz == 13000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x6BE2115/*  7A: 175280 */, 0x6CF24B4/*  7B: 177008 */, 0x6E02854/*  7C: 178736 */, 
	0x6F934C6/*  8A: 181280 */, 0x70A3866/*  8B: 183008 */, 0x71B3C05/*  8C: 184736 */, 
	0x7344877/*  9A: 187280 */, 0x7454C17/*  9B: 189008 */, 0x7564FB6/*  9C: 190736 */, 
	0x76F5C28/* 10A: 193280 */, 0x7805FC8/* 10B: 195008 */, 0x7916368/* 10C: 196736 */, 
	0x7AA6FDA/* 11A: 199280 */, 0x7BB7379/* 11B: 201008 */, 0x7CC7719/* 11C: 202736 */, 
	0x7E5838B/* 12A: 205280 */, 0x7F6872B/* 12B: 207008 */, 0x8078ACA/* 12C: 208736 */, 
	0x820973C/* 13A: 211280 */, 0x8319ADC/* 13B: 213008 */, 0x8429E7B/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 16000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x57A7AE1/*  7A: 175280 */, 0x5884DD2/*  7B: 177008 */, 0x59620C4/*  7C: 178736 */, 
	0x5AA7AE1/*  8A: 181280 */, 0x5B84DD2/*  8B: 183008 */, 0x5C620C4/*  8C: 184736 */, 
	0x5DA7AE1/*  9A: 187280 */, 0x5E84DD2/*  9B: 189008 */, 0x5F620C4/*  9C: 190736 */, 
	0x60A7AE1/* 10A: 193280 */, 0x6184DD2/* 10B: 195008 */, 0x62620C4/* 10C: 196736 */, 
	0x63A7AE1/* 11A: 199280 */, 0x6484DD2/* 11B: 201008 */, 0x65620C4/* 11C: 202736 */, 
	0x66A7AE1/* 12A: 205280 */, 0x6784DD2/* 12B: 207008 */, 0x68620C4/* 12C: 208736 */, 
	0x69A7AE1/* 13A: 211280 */, 0x6A84DD2/* 13B: 213008 */, 0x6B620C4/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 16384)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x5599C00/*  7A: 175280 */, 0x5671C00/*  7B: 177008 */, 0x5749C00/*  7C: 178736 */, 
	0x5887C00/*  8A: 181280 */, 0x595FC00/*  8B: 183008 */, 0x5A37C00/*  8C: 184736 */, 
	0x5B75C00/*  9A: 187280 */, 0x5C4DC00/*  9B: 189008 */, 0x5D25C00/*  9C: 190736 */, 
	0x5E63C00/* 10A: 193280 */, 0x5F3BC00/* 10B: 195008 */, 0x6013C00/* 10C: 196736 */, 
	0x6151C00/* 11A: 199280 */, 0x6229C00/* 11B: 201008 */, 0x6301C00/* 11C: 202736 */, 
	0x643FC00/* 12A: 205280 */, 0x6517C00/* 12B: 207008 */, 0x65EFC00/* 12C: 208736 */, 
	0x672DC00/* 13A: 211280 */, 0x6805C00/* 13B: 213008 */, 0x68DDC00/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 18000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x4DEA61D/*  7A: 175280 */, 0x4EAEFD7/*  7B: 177008 */, 0x4F73992/*  7C: 178736 */, 
	0x50950C8/*  8A: 181280 */, 0x5159A82/*  8B: 183008 */, 0x521E43C/*  8C: 184736 */, 
	0x533FB72/*  9A: 187280 */, 0x540452D/*  9B: 189008 */, 0x54C8EE7/*  9C: 190736 */, 
	0x55EA61D/* 10A: 193280 */, 0x56AEFD7/* 10B: 195008 */, 0x5773992/* 10C: 196736 */, 
	0x58950C8/* 11A: 199280 */, 0x5959A82/* 11B: 201008 */, 0x5A1E43C/* 11C: 202736 */, 
	0x5B3FB72/* 12A: 205280 */, 0x5C0452D/* 12B: 207008 */, 0x5CC8EE7/* 12C: 208736 */, 
	0x5DEA61D/* 13A: 211280 */, 0x5EAEFD7/* 13B: 213008 */, 0x5F73992/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 19200)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x490BBBB/*  7A: 175280 */, 0x49C40DA/*  7B: 177008 */, 0x4A7C5F9/*  7C: 178736 */, 
	0x4B8BBBB/*  8A: 181280 */, 0x4C440DA/*  8B: 183008 */, 0x4CFC5F9/*  8C: 184736 */, 
	0x4E0BBBB/*  9A: 187280 */, 0x4EC40DA/*  9B: 189008 */, 0x4F7C5F9/*  9C: 190736 */, 
	0x508BBBB/* 10A: 193280 */, 0x51440DA/* 10B: 195008 */, 0x51FC5F9/* 10C: 196736 */, 
	0x530BBBB/* 11A: 199280 */, 0x53C40DA/* 11B: 201008 */, 0x547C5F9/* 11C: 202736 */, 
	0x558BBBB/* 12A: 205280 */, 0x56440DA/* 12B: 207008 */, 0x56FC5F9/* 12C: 208736 */, 
	0x580BBBB/* 13A: 211280 */, 0x58C40DA/* 13B: 213008 */, 0x597C5F9/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 24000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x3A6FC96/*  7A: 175280 */, 0x3B033E1/*  7B: 177008 */, 0x3B96B2D/*  7C: 178736 */, 
	0x3C6FC96/*  8A: 181280 */, 0x3D033E1/*  8B: 183008 */, 0x3D96B2D/*  8C: 184736 */, 
	0x3E6FC96/*  9A: 187280 */, 0x3F033E1/*  9B: 189008 */, 0x3F96B2D/*  9C: 190736 */, 
	0x406FC96/* 10A: 193280 */, 0x41033E1/* 10B: 195008 */, 0x4196B2D/* 10C: 196736 */, 
	0x426FC96/* 11A: 199280 */, 0x43033E1/* 11B: 201008 */, 0x4396B2D/* 11C: 202736 */, 
	0x446FC96/* 12A: 205280 */, 0x45033E1/* 12B: 207008 */, 0x4596B2D/* 12C: 208736 */, 
	0x466FC96/* 13A: 211280 */, 0x47033E1/* 13B: 213008 */, 0x4796B2D/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 24576)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x39112AA/*  7A: 175280 */, 0x39A12AA/*  7B: 177008 */, 0x3A312AA/*  7C: 178736 */, 
	0x3B052AA/*  8A: 181280 */, 0x3B952AA/*  8B: 183008 */, 0x3C252AA/*  8C: 184736 */, 
	0x3CF92AA/*  9A: 187280 */, 0x3D892AA/*  9B: 189008 */, 0x3E192AA/*  9C: 190736 */, 
	0x3EED2AA/* 10A: 193280 */, 0x3F7D2AA/* 10B: 195008 */, 0x400D2AA/* 10C: 196736 */, 
	0x40E12AA/* 11A: 199280 */, 0x41712AA/* 11B: 201008 */, 0x42012AA/* 11C: 202736 */, 
	0x42D52AA/* 12A: 205280 */, 0x43652AA/* 12B: 207008 */, 0x43F52AA/* 12C: 208736 */, 
	0x44C92AA/* 13A: 211280 */, 0x45592AA/* 13B: 213008 */, 0x45E92AA/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 26000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x35F108A/*  7A: 175280 */, 0x367925A/*  7B: 177008 */, 0x370142A/*  7C: 178736 */, 
	0x37C9A63/*  8A: 181280 */, 0x3851C33/*  8B: 183008 */, 0x38D9E02/*  8C: 184736 */, 
	0x39A243B/*  9A: 187280 */, 0x3A2A60B/*  9B: 189008 */, 0x3AB27DB/*  9C: 190736 */, 
	0x3B7AE14/* 10A: 193280 */, 0x3C02FE4/* 10B: 195008 */, 0x3C8B1B4/* 10C: 196736 */, 
	0x3D537ED/* 11A: 199280 */, 0x3DDB9BC/* 11B: 201008 */, 0x3E63B8C/* 11C: 202736 */, 
	0x3F2C1C5/* 12A: 205280 */, 0x3FB4395/* 12B: 207008 */, 0x403C565/* 12C: 208736 */, 
	0x4104B9E/* 13A: 211280 */, 0x418CD6E/* 13B: 213008 */, 0x4214F3D/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 27000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x33F1969/*  7A: 175280 */, 0x3474A8F/*  7B: 177008 */, 0x34F7BB6/*  7C: 178736 */, 
	0x35B8B30/*  8A: 181280 */, 0x363BC57/*  8B: 183008 */, 0x36BED7D/*  8C: 184736 */, 
	0x377FCF7/*  9A: 187280 */, 0x3802E1E/*  9B: 189008 */, 0x3885F45/*  9C: 190736 */, 
	0x3946EBE/* 10A: 193280 */, 0x39C9FE5/* 10B: 195008 */, 0x3A4D10C/* 10C: 196736 */, 
	0x3B0E085/* 11A: 199280 */, 0x3B911AC/* 11B: 201008 */, 0x3C142D3/* 11C: 202736 */, 
	0x3CD524C/* 12A: 205280 */, 0x3D58373/* 12B: 207008 */, 0x3DDB49A/* 12C: 208736 */, 
	0x3E9C413/* 13A: 211280 */, 0x3F1F53A/* 13B: 213008 */, 0x3FA2661/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 32000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x2BD3D70/*  7A: 175280 */, 0x2C426E9/*  7B: 177008 */, 0x2CB1062/*  7C: 178736 */, 
	0x2D53D70/*  8A: 181280 */, 0x2DC26E9/*  8B: 183008 */, 0x2E31062/*  8C: 184736 */, 
	0x2ED3D70/*  9A: 187280 */, 0x2F426E9/*  9B: 189008 */, 0x2FB1062/*  9C: 190736 */, 
	0x3053D70/* 10A: 193280 */, 0x30C26E9/* 10B: 195008 */, 0x3131062/* 10C: 196736 */, 
	0x31D3D70/* 11A: 199280 */, 0x32426E9/* 11B: 201008 */, 0x32B1062/* 11C: 202736 */, 
	0x3353D70/* 12A: 205280 */, 0x33C26E9/* 12B: 207008 */, 0x3431062/* 12C: 208736 */, 
	0x34D3D70/* 13A: 211280 */, 0x35426E9/* 13B: 213008 */, 0x35B1062/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 32768)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x2ACCE00/*  7A: 175280 */, 0x2B38E00/*  7B: 177008 */, 0x2BA4E00/*  7C: 178736 */, 
	0x2C43E00/*  8A: 181280 */, 0x2CAFE00/*  8B: 183008 */, 0x2D1BE00/*  8C: 184736 */, 
	0x2DBAE00/*  9A: 187280 */, 0x2E26E00/*  9B: 189008 */, 0x2E92E00/*  9C: 190736 */, 
	0x2F31E00/* 10A: 193280 */, 0x2F9DE00/* 10B: 195008 */, 0x3009E00/* 10C: 196736 */, 
	0x30A8E00/* 11A: 199280 */, 0x3114E00/* 11B: 201008 */, 0x3180E00/* 11C: 202736 */, 
	0x321FE00/* 12A: 205280 */, 0x328BE00/* 12B: 207008 */, 0x32F7E00/* 12C: 208736 */, 
	0x3396E00/* 13A: 211280 */, 0x3402E00/* 13B: 213008 */, 0x346EE00/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 36000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x26F530E/*  7A: 175280 */, 0x27577EB/*  7B: 177008 */, 0x27B9CC9/*  7C: 178736 */, 
	0x284A864/*  8A: 181280 */, 0x28ACD41/*  8B: 183008 */, 0x290F21E/*  8C: 184736 */, 
	0x299FDB9/*  9A: 187280 */, 0x2A02296/*  9B: 189008 */, 0x2A64773/*  9C: 190736 */, 
	0x2AF530E/* 10A: 193280 */, 0x2B577EB/* 10B: 195008 */, 0x2BB9CC9/* 10C: 196736 */, 
	0x2C4A864/* 11A: 199280 */, 0x2CACD41/* 11B: 201008 */, 0x2D0F21E/* 11C: 202736 */, 
	0x2D9FDB9/* 12A: 205280 */, 0x2E02296/* 12B: 207008 */, 0x2E64773/* 12C: 208736 */, 
	0x2EF530E/* 13A: 211280 */, 0x2F577EB/* 13B: 213008 */, 0x2FB9CC9/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 38400)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x2485DDD/*  7A: 175280 */, 0x24E206D/*  7B: 177008 */, 0x253E2FC/*  7C: 178736 */, 
	0x25C5DDD/*  8A: 181280 */, 0x262206D/*  8B: 183008 */, 0x267E2FC/*  8C: 184736 */, 
	0x2705DDD/*  9A: 187280 */, 0x276206D/*  9B: 189008 */, 0x27BE2FC/*  9C: 190736 */, 
	0x2845DDD/* 10A: 193280 */, 0x28A206D/* 10B: 195008 */, 0x28FE2FC/* 10C: 196736 */, 
	0x2985DDD/* 11A: 199280 */, 0x29E206D/* 11B: 201008 */, 0x2A3E2FC/* 11C: 202736 */, 
	0x2AC5DDD/* 12A: 205280 */, 0x2B2206D/* 12B: 207008 */, 0x2B7E2FC/* 12C: 208736 */, 
	0x2C05DDD/* 13A: 211280 */, 0x2C6206D/* 13B: 213008 */, 0x2CBE2FC/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 40000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x230FDF3/*  7A: 175280 */, 0x2368587/*  7B: 177008 */, 0x23C0D1B/*  7C: 178736 */, 
	0x2443126/*  8A: 181280 */, 0x249B8BA/*  8B: 183008 */, 0x24F404E/*  8C: 184736 */, 
	0x257645A/*  9A: 187280 */, 0x25CEBED/*  9B: 189008 */, 0x2627381/*  9C: 190736 */, 
	0x26A978D/* 10A: 193280 */, 0x2701F21/* 10B: 195008 */, 0x275A6B5/* 10C: 196736 */, 
	0x27DCAC0/* 11A: 199280 */, 0x2835254/* 11B: 201008 */, 0x288D9E8/* 11C: 202736 */, 
	0x290FDF3/* 12A: 205280 */, 0x2968587/* 12B: 207008 */, 0x29C0D1B/* 12C: 208736 */, 
	0x2A43126/* 13A: 211280 */, 0x2A9B8BA/* 13B: 213008 */, 0x2AF404E/* 13C: 214736 */
	
  };

#elif (RTV_SRC_CLK_FREQ_KHz == 48000)
  static const U32 g_atPllNF_TDMB_Korea[] = 
  {
	0x1D37E4B/*  7A: 175280 */, 0x1D819F0/*  7B: 177008 */, 0x1DCB596/*  7C: 178736 */, 
	0x1E37E4B/*  8A: 181280 */, 0x1E819F0/*  8B: 183008 */, 0x1ECB596/*  8C: 184736 */, 
	0x1F37E4B/*  9A: 187280 */, 0x1F819F0/*  9B: 189008 */, 0x1FCB596/*  9C: 190736 */, 
	0x2037E4B/* 10A: 193280 */, 0x20819F0/* 10B: 195008 */, 0x20CB596/* 10C: 196736 */, 
	0x2137E4B/* 11A: 199280 */, 0x21819F0/* 11B: 201008 */, 0x21CB596/* 11C: 202736 */, 
	0x2237E4B/* 12A: 205280 */, 0x22819F0/* 12B: 207008 */, 0x22CB596/* 12C: 208736 */, 
	0x2337E4B/* 13A: 211280 */, 0x23819F0/* 13B: 213008 */, 0x23CB596/* 13C: 214736 */
	
  };

#else
	#error "Code not present"
#endif
