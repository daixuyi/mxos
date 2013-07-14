/**
 ******************************************************************************
 * @file      oshook.c
 * @brief     os������ṩ�Ĺ��ӽӿڿ�.
 * @details   This file including all API functions's  implement of mxhook.c.
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <oscfg.h>
#if CORE_TYPE == CORE_CM3
#include <core_cm3.h>
#elif  CORE_TYPE == CORE_CM4
#include <core_cm4.h>
#endif

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
/**
 *  @brief ιӲ������������
 */
FUNCPTR     _func_feedDogHook = NULL;

/**
 *  @brief оƬ��λ��������
 */
FUNCPTR     _func_cpuRestHook = NULL;

/**
 *  @brief ����ʱδι���쳣ʱ��������
 */
FUNCPTR     _func_dmnRestHook = NULL;

/**
 *  @brief �ں������ջ�������ʱ�����õĹ��Ӻ�����
 *  ���������ļ��б����ֳ���Ϣ��
 *  _func_evtLogOverStackHook(taskId,pcTaskName);
 */
VOIDFUNCPTR _func_evtLogOverStackHook;

int32_t _the_console_fd = -1;

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Local Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief   0. Ĭ�Ϲرտ��Ź�
 * @param[in]  None
 *
 * @retrun    None
 ******************************************************************************
 */
void
_default_bsp_close_watchdog(void)
{
    //�պ���
}

/**
 ******************************************************************************
 * @brief   1. Ĭ��uart�ײ�����ֽ�
 * @param[in]  None
 *
 * @retval    None
 ******************************************************************************
 */
void
_default_bsp_putchar(char_t c)
{

}

/**
 ******************************************************************************
 * @brief   2. Ĭ��uart�ײ��ȡ�ֽ�,��ѯģʽ���������ݷ���0
 * @param[in]  None
 *
 * @retval    0 :
 ******************************************************************************
 */
int32_t
_default_bsp_getchar(void)
{
    return 0;
}

/**
 ******************************************************************************
 * @brief   3. Ĭ��bsp�ײ�ʵ�ָ�λ����
 * @param[in]  None
 *
 * @retval    None
 ******************************************************************************
 */
void
_default_bsp_reboot(void)
{
    __asm volatile ("isb");
    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
                   (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                   SCB_AIRCR_SYSRESETREQ_Msk);
    __asm volatile ("dsb");
    while(1);
}

/**
 ******************************************************************************
 * @brief  4. Ĭ��������ʱ��
 * @param[in]  None
 *
 * @retval    None
 ******************************************************************************
 */
void
_default_bsp_timer_start(void)
{

}

/**
 ******************************************************************************
 * @brief  4. Ĭ�ϻ�ȡ������ֵ
 * @param[in]  None
 *
 * @retval    0
 ******************************************************************************
 */
uint32_t
_default_bsp_timer_get(void)
{
    return 0;
}

/**
 ******************************************************************************
 * @brief  5. Ĭ�ϻ�ȡMCU��Ƶ
 * @param[in]  None
 *
 * @retval    0
 ******************************************************************************
 */
uint32_t
_default_bsp_mcu_clk(void)
{
    return 0;
}

/**
 ******************************************************************************
 * @brief  6. Ĭ�ϻ�ȡMCU�ж�����
 * @param[in]  None
 *
 * @retval    0
 ******************************************************************************
 */
uint32_t
_default_bsp_get_max_int_count(void)
{
    return 103;
}

/**
  *@brief MXOSĬ�Ϲ��Ӻ���
  */
#pragma weak bsp_close_watchdog     = _default_bsp_close_watchdog
#pragma weak bsp_putchar            = _default_bsp_putchar
#pragma weak bsp_getchar            = _default_bsp_getchar
#pragma weak bsp_reboot             = _default_bsp_reboot
#pragma weak bsp_timer_start        = _default_bsp_timer_start
#pragma weak bsp_timer_get          = _default_bsp_timer_get
#pragma weak bsp_get_mcu_clk        = _default_bsp_mcu_clk
#pragma weak bsp_get_max_int_count  = _default_bsp_get_max_int_count


/*--------------------------------oshook.c-----------------------------------*/
