/*         �������������ϰ
  û�к����Զ�ע���豸����Ҫ��װ�������ֶ�ע��
*/



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


#define DEVICE_NAME     "hello"  /* ����ģʽ��ִ�С�cat /proc/devices����������豸���� */
#define HELLO_MAJOR       231     /* �ֶ��������豸�ţ� �����Ϊ0,��ʾ�Զ��������豸�� */




/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations hello_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
};

/*
 * ִ�С�insmod hello.koʱ�ͻ�����������
 */
static int __init hello_init(void)
{
    int ret;

    /* ע���ַ��豸��������
     * ����Ϊ���豸�š��豸���֡�file_operations�ṹ��
     * ���������豸�žͺ;����file_operations�ṹ��ϵ�����ˣ�
     * �������豸ΪHELLO_MAJOR���豸�ļ�ʱ���ͻ����hello_fops�е���س�Ա����
     * HELLO_MAJOR������Ϊ0����ʾ���ں��Զ��������豸��
     */
    ret = register_chrdev(HELLO_MAJOR, DEVICE_NAME, &hello_fops);//��װ����
    if (ret < 0) {
      printk(DEVICE_NAME "��װ����ʧ�ܣ�\n");
      return ret;
    }
    
    printk(DEVICE_NAME "Hello��\n");
    return 0;
}

/*
 * ִ�С�rmmod hello.ko������ʱ�ͻ����������� 
 */
static void __exit hello_exit(void)
{
    /* ж���������� */
    unregister_chrdev(HELLO_MAJOR, DEVICE_NAME);
}


/* ������ָ����������ĳ�ʼ��������ж�غ��� */
module_init(hello_init);
module_exit(hello_exit);

/* �������������һЩ��Ϣ�����Ǳ���� */
MODULE_AUTHOR("lijunjun");             // �������������
MODULE_DESCRIPTION("hello Driver");   // һЩ������Ϣ
/*��ѭ��Э��,����*/
MODULE_LICENSE("GPL");                              // ��ѭ��Э��


