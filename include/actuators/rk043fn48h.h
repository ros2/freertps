/**
  ******************************************************************************
  * @file    rk043fn48h.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    25-June-2015
  * @brief   This file contains all the constants parameters for the RK043FN48H-CT672B
  *          LCD component.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RK043FN48H_H
#define __RK043FN48H_H

#define  LCD_DISPLAY_WIDTH    ((uint16_t)480)          /* LCD PIXEL WIDTH            */
#define  LCD_DISPLAY_HEIGHT   ((uint16_t)272)          /* LCD PIXEL HEIGHT           */

/** 
  * @brief  LCD_DISPLAY Timing  
  */     
#define  LCD_DISPLAY_HSYNC            ((uint16_t)41)   /* Horizontal synchronization */
#define  LCD_DISPLAY_HBP              ((uint16_t)13)   /* Horizontal back porch      */
#define  LCD_DISPLAY_HFP              ((uint16_t)32)   /* Horizontal front porch     */
#define  LCD_DISPLAY_VSYNC            ((uint16_t)10)   /* Vertical synchronization   */
#define  LCD_DISPLAY_VBP              ((uint16_t)2)    /* Vertical back porch        */
#define  LCD_DISPLAY_VFP              ((uint16_t)2)    /* Vertical front porch       */

/** 
  * @brief  LCD_DISPLAY frequency divider  
  */    
#define  LCD_DISPLAY_FREQUENCY_DIVIDER    5            /* LCD Frequency divider      */
#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
