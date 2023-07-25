1、前言
    电源管理的基本行为：
    a, 实时的关闭暂时不使用的部分（可称作“工作状态到非工作状态的转移”）。
    b, 当需要重新使用那些已关闭部分时（可称作“非工作状态到工作状态的转移”），不能有太长时间的等待，且转移过程不能消耗太多的能量。

2、Linux电源管理的组成
    电源管理（Power Management）在Linux Kernel中，是一个比较庞大的子系统，涉及到供电（Power Supply）、充电（Charger）、时钟（Clock）、频率（Frequency）、
电压（Voltage）、睡眠/唤醒（Suspend/Resume）等方方面面。

    基本概念:
    Power Supply，是一个供用户空间程序监控系统的供电状态（电池供电、USB供电、AC供电等等）的class。通俗的讲，它是一个Battery&Charger驱动的Framework
    Clock Framework，Clock驱动的Framework，用于统一管理系统的时钟资源
    Regulator Framework，Voltage/Current Regulator驱动的Framework。该驱动用于调节CPU等模块的电压和电流值
    Dynamic Tick/Clock Event，在传统的Linux Kernel中，系统Tick是固定周期（如10ms）的，因此每隔一个Tick，就会产生一个Timer中断。这会唤醒处于Idle或者Sleep状
态的CPU，而很多时候这种唤醒是没有意义的。因此新的Kernel就提出了Dynamic Tick的概念，Tick不再是周期性的，而是根据系统中定时器的情况，不规律的产生，这样可以减少很
多无用的Timer中断
    CPU Idle，用于控制CPU Idle状态的Framework
    Generic PM，传统意义上的Power Management，如Power Off、Suspend to RAM、Suspend to Disk、Hibernate等
    Runtime PM and Wakelock，运行时的Power Management，不再需要用户程序的干涉，由Kernel统一调度，实时的关闭或打开设备，以便在使用性能和省电性能之间找到最佳的
平衡
    /* Runtime PM是Linux Kernel亲生的运行时电源管理机制，Wakelock是由Android提出的机制。这两种机制的目的是一样的，因此只需要支持一种即可。另外，由于Wakelock
机制路子太野了，饱受Linux社区的鄙视，因此我们不会对该机制进行太多的描述。*/
    CPU Freq/Device Freq，用于实现CPU以及Device频率调整的Framework
    OPP（Operating Performance Point），是指可以使SOCs或者Devices正常工作的电压和频率组合。内核提供这一个Layer，是为了在众多的电压和频率组合中，筛选出一些相对
固定的组合，从而使事情变得更为简单一些
    PM QOS，所谓的PM QOS，是指系统在指定的运行状态下（不同电压、频率，不同模式之间切换，等等）的工作质量，包括latency、timeout、throughput三个参数，单位分别为
us、us和kb/s。通过QOS参数，可以分析、改善系统的性能

3、kernel 中电源管理相关的源码
kernel/power/ *
drivers/power/
drivers/base/power/ *
drivers/cpuidle/ *
drivers/cpufreq/ *
drivers/devfreq/ *
include/linux/power_supply.h
include/linux/cpuidle.h
include/linux/cpufreq.h
include/linux/cpu_pm.h
include/linux/device.h
include/linux/pm.h
include/linux/pm domain.h
include/linux/pm runtime.h
include/linux/pm wakeup.h
include/linux/suspend.h
Documentation/power/ *.txt