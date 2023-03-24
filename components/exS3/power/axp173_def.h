#ifndef _AXP173_DEF_H_
#define _AXP173_DEF_H_


/* ========= enable command ============= */
//        [12]reg [3] [4]bit_num 
#define EN_OP_EXTEN (0x1002)
#define EN_OP_DC2   (0x1000)
#define EN_OP_LDO3  (0x1203)  
#define EN_OP_LDO2  (0x1202)
#define EN_OP_LDO4  (0x1201)
#define EN_OP_DC1   (0x1200)

#define EN_VHOLD_LIMIT          (0x3006)
#define EN_VBUS_CURRENT_LIMIT   (0x3001)
#define EN_CHARGE               (0x3307)
#define EN_PEK_SHUTDOWN         (0x3603)

#define EN_ADC_BAT_VOLT     (0x8207)
#define EN_ADC_BAT_CURRENT  (0x8206)
#define EN_ADC_ACIN_VOLT    (0x8205)
#define EN_ADC_ACIN_CURRENT (0x8204)
#define EN_ADC_VBUS_VOLT    (0x8203)
#define EN_ADC_VBUS_CURRENT (0x8202)
#define EN_ADC_APS_VOLT     (0x8201)
#define EN_ADC_TS_PIN       (0x8200)
#define EN_ADC_INTER_TEMP   (0x8307)

#define EN_COLUMB   (0xb807)
/* ------------------------------------- */



/* ========= volt setting ============= */
//   [12]reg [3]bit_num [4]bit_length
#define DC2_SET_VOLT    (0x2305)  // 700~2275, 25mV/step
#define DC1_SET_VOLT    (0x2606)  // 700~3500, 25mV/step
#define LDO4_SET_VOLT   (0x2706)  // 700~3500, 25mV/step
#define LDO2_SET_VOLT   (0x2844)  // 1800~3300, 100mV/step   
#define LDO3_SET_VOLT   (0x2804)  // 1800~3300, 100mV/step   
#define VHOLD_SET_VOLT  (0x3033)  // 4000~4700, 100mV/step
#define VOFF_SET_VOLT   (0x3103)  // 2600~3300, 100mV/step
/* ------------------------------------- */



/* ========= ADC data ============= */
//   [12]reg [3]bit_num [4]bit_length
#define DATA_ACIN_VOLT      (0x5602)
#define DATA_ACIN_CURRENT   (0x5802)
#define DATA_VBUS_VOLT      (0x5a02)
#define DATA_VBUS_CURRENT   (0x5c02)
#define DATA_INTEL_TEMP     (0x5e02)
#define DATA_TS_ADC         (0x6202)

#define DATA_BAT_POWER      (0x7003)
#define DATA_BAT_VOLT       (0x7802)
#define DATA_BAT_CHARGE_CURRENT     (0x7a02)
#define DATA_BAT_DISCHARGE_CURRENT  (0x7c02)

#define DATA_APS_VOLT       (0x7e02)

// #define DATA_COLUMB_CHARGE      (0xb004)
// #define DATA_COLUMB_DISCHARGE   (0xb404)
/* -------------------------------- */




/* registers */
#define AXP173_PEK              (0x36)  //PEK 参数设置寄存器

/* Interrupt control registers */
#define AXP173_IRQ_EN_CONTROL_1 (0x40)  //IRQ 使能控制寄存器 1
#define AXP173_IRQ_EN_CONTROL_2 (0x41)  //IRQ 使能控制寄存器 2
#define AXP173_IRQ_EN_CONTROL_3 (0x42)  //IRQ 使能控制寄存器 3
#define AXP173_IRQ_EN_CONTROL_4 (0x43)  //IRQ 使能控制寄存器 4

#define AXP173_IRQ_STATUS_1     (0x44)  //IRQ 状态寄存器 1
#define AXP173_IRQ_STATUS_2     (0x45)  //IRQ 状态寄存器 2
#define AXP173_IRQ_STATUS_3     (0x46)  //IRQ 状态寄存器 3
#define AXP173_IRQ_STATUS_4     (0x47)  //IRQ 状态寄存器 4


#define AXP173_COULOMB_COUNTER_CONTROL  (0xb8)  //库仑计控制寄存器
#define AXP173_COULOMB_COUNTER          (0xff)  /* Computed ADC */



#endif

