#include "interface.h"
#include "interface_collector.h"

#ifdef INTERFACE_STATIC_MODE
static interface_t interface_pool[INTERFACE_STATIC_MAX_INTERFACES];
#else
static interface_t interface_root;
static int lastIntefaceDesc = 0;
#endif /* INTERFACE_STATIC_MODE */

//======================================================================================================================
#ifdef INTERFACE_STATIC_MODE
/*!
 * \brief   Функция поиска свободного дескриптора в пуле интерфейсов
 * \param   None
 * \retval  возвращает номер дескрипора или -1, если свободных дескрипторов нет
 */
static int lookup_free_interface_descriptor (void)
{
    int i;
    for (i = 0; i < INTERFACE_STATIC_MAX_INTERFACES; i++)
        if (interface_pool[i].type == IF_TYPES_NONE) break;

    if (i == INTERFACE_STATIC_MAX_INTERFACES) return -1;
    return i;
}
#endif /* INTERFACE_STATIC_MODE */

/*!
 * \brief   Функция поиска дескриптора в цепочке по его идентификатору
 * \param   None
 * \retval  Указатель на заказанный экземпляр интерфейса или NULL, если дескпритора с таким номером не найдено
 */
static interface_t* getInterface_by_desc (int desc)
{
    interface_t *iface = NULL;
#ifdef INTERFACE_STATIC_MODE
    if (desc >= INTERFACE_STATIC_MAX_INTERFACES)
    {
        return iface;
    }
    else
    {
        iface = &interface_pool[desc];
    }
#else
    iface = &interface_root;

    while (iface->next != 0)
    {
        if (iface->desc == desc) break;
        iface = iface->next;
    }
#endif /* INTERFACE_STATIC_MODE */
    return iface;
}


//======================================================================================================================

void interface_init (void)
{
#ifdef INTERFACE_STATIC_MODE
    memset (&interface_pool, 0x00, sizeof (interface_pool));
#else
    memset (&interface_root, 0x00, sizeof (interface_root));
#endif /* INTERFACE_STATIC_MODE */

   // Собираем битовые маски о включенных интерфейсах
   interfaces_collect();
}

//======================================================================================================================
/*!
 * \brief Функция открытия интерфейса
 * \param[in] ifname - строка с именем интерфейса.
 * \param[in] flags - флаги открытия интерфейса. Оставлен для совместимости с POSIX стандартом.
 * Ни на что не вляет
 * \return Возвращает номер идентификатора открытого интерфейса, через который в дальнейшем и ведется работа
 */
int open (char* ifname, int flags)
{
    int i;
    interface_t *if_new, if_temp;
    interface_types_t iftype;

    // Проверяем - есть ли запрашиваемый интерфейс в системе
    iftype = interface_assign(ifname, &if_temp);
    if (iftype == IF_TYPES_NONE)
    {
        // Нет такого - возвращаем минус единицу
        return -1;
    }

    // Надо же, интерфейс нашелся. Тогда или ищем свободный номер в пуле,
    // или выделяем память под вновь открытый интерфейс
#ifdef INTERFACE_STATIC_MODE
    i = lookup_free_interface_descriptor();
    if (i < 0) return i;
    if_new = &interface_pool[i];
#else
    interface_t *p = &interface_root;
    while (p->next != 0)
    {
        p = p->next;
    }
    if_new = malloc (sizeof(interface_t));
    if (if_new == 0) return -1;
    p->next = if_new;
    i = ++lastIntefaceDesc;
#endif /* INTERFACE_STATIC_MODE */

    // интерфейс в системе есть, место и прочее для него выделено - копируем и возвращаем номер дескриптора
    memcpy (if_new, &if_temp, sizeof (interface_t));

    // Задаем номера дескрипторов
    if_new->next = NULL;
    if_new->desc = i;
    memset(&if_new->options, 0x00, sizeof (struct termios));

    return i;
}

void close (int desc)
{
    interface_t *iface = getInterface_by_desc(desc);
    if (iface == NULL) return;

#ifdef INTERFACE_STATIC_MODE
    memset ((void*)iface, 0x00, sizeof (interface_t));
#else
    interface_t *p;
    if (iface)
    {
        // Найдем в цепочке интерфейс, который ссылается на этот, который надо удалить
        p = &interface_root;
        while ((p->next != iface) && (p->next != 0))
        {
            p = p->next;
        }

        if (p)
        {
            p->next = iface->next;
            free (iface);
        }
    }
#endif /* INTERFACE_STATIC_MODE */
}

ssize_t read (int desc, char* buffer, size_t size)
{
    const interface_t *iface;
    ssize_t len;

    iface = getInterface_by_desc(desc);
    if (iface == NULL) return -1;

    if (iface->type == IF_TYPES_NONE) return -1;
    len = iface->syscalls.read(buffer, size);
    return len;
}

ssize_t write (int desc, char* buffer, size_t size)
{
    const interface_t *iface;
    ssize_t len;

    iface = getInterface_by_desc(desc);
    if (iface == NULL) return -1;

    if (iface->type == IF_TYPES_NONE) return -1;
    len = iface->syscalls.write(buffer, size);
    return len;
}

int poll (int desc)
{
    const interface_t *iface;

    iface = getInterface_by_desc(desc);
    if (iface == NULL) return -1;

    if (iface->type == IF_TYPES_NONE) return -1;
    iface->syscalls.poll();

    return 0;
}

int tcgetattr (int fd, struct termios *opt)
{
    const interface_t *iface;

    iface = getInterface_by_desc(fd);
    if (iface == NULL) return -1;

    memcpy ((void*)opt, (void*)&iface->options, sizeof (struct termios));
    return 0;
}

int tcsetattr (int fd, int optional, struct termios *opt)
{
    const interface_t *iface;

    iface = getInterface_by_desc(fd);
    if (iface == NULL) return -1;

    memcpy ((void*)&iface->options, opt, sizeof(struct termios));

    switch (iface->type)
    {
#ifdef INTERFACE_UART
        case IF_TYPES_UART:
            // Установим скорость
            uart_init(iface->handler.uart, opt);
            break;
#endif // INTERFACE_UART

#ifdef INTERFACE_SPI
        case IF_TYPES_SPI:
            spi_device_init(iface->handler.spi, opt);
            break;
#endif // INTERFACE_SPI

#ifdef INTERFACE_I2C
        case IF_TYPES_I2C:
            i2c_device_init(iface->handler.i2c, opt);
            break;
#endif // INTERFACE_I2C

#ifdef INTERFACE_SX1276_LORA
        case IF_TYPES_SX1276_LORA:
            sx1276_lora_device_init(iface->handler.sx1276_lora, opt);
            break;
#endif // INTERFACE_SX_1276_LORA

#ifdef INTERFACE_SX1276_FSK
        case IF_TYPES_SX1276_FSK:
            sx1276_fsk_device_init(iface->handler.sx1276_fsk, opt);
            break;
#endif // INTERFACE_SX_1276_FSK

        case IF_TYPES_NONE:
        default:
            return -1;

    }

    return 0;
}

int tcsetiospeed (struct termios *tc, speed_t baud)
{
    tc->c_ospeed = baud;
    tc->c_ispeed = baud;

    return 0;
}

int interface_switch_set_to_tx_func (int fd, void (*switch_func)(void))
{
    interface_t *iface;
    int res = 0;

    iface = getInterface_by_desc(fd);

    if (iface == NULL) return -1;

    switch (iface->type)
    {
#ifdef INTERFACE_SX1276_LORA
        case IF_TYPES_SX1276_LORA:
            iface->handler.sx1276_lora->transc.ll_func.switch_to_tx = switch_func;
            break;
#endif
#ifdef INTERFACE_SX1276_FSK
        case IF_TYPES_SX1276_FSK:
            iface->handler.sx1276_fsk->transc.ll_func.switch_to_tx = switch_func;
            break;
#endif
        default:
            res = -1;
            break;
    }

    return res;
}

int interface_switch_set_to_rx_func (int fd, void (*switch_func)(void))
{
    interface_t *iface;
    int res = 0;

    iface = getInterface_by_desc(fd);

    if (iface == NULL) return -1;

    switch (iface->type)
    {
#ifdef INTERFACE_SX1276_LORA
        case IF_TYPES_SX1276_LORA:
            iface->handler.sx1276_lora->transc.ll_func.switch_to_rx = switch_func;
            break;
#endif
#ifdef INTERFACE_SX1276_FSK
            case IF_TYPES_SX1276_FSK:
            iface->handler.sx1276_fsk->transc.ll_func.switch_to_rx = switch_func;
            break;
#endif
        default:
            res = -1;
            break;
    }

    return res;
}

int interface_reswitch_set_to_tx_func (int fd)
{
    interface_t *iface;
    int res = 0;

    iface = getInterface_by_desc(fd);

    if (iface == NULL) return -1;

    switch (iface->type)
    {
#ifdef INTERFACE_SX1276_LORA
        case IF_TYPES_SX1276_LORA:
            iface->handler.sx1276_lora->transc.ll_func.switch_to_tx = NULL;
            break;
#endif
#ifdef INTERFACE_SX1276_FSK
        case IF_TYPES_SX1276_FSK:
            iface->handler.sx1276_fsk->transc.ll_func.switch_to_tx = NULL;
            break;
#endif
        default:
            res = -1;
            break;
    }

    return res;
}

int interface_reswitch_set_to_rx_func (int fd)
{
    interface_t *iface;
    int res = 0;

    iface = getInterface_by_desc(fd);

    if (iface == NULL) return -1;

    switch (iface->type)
    {
#ifdef INTERFACE_SX1276_LORA
        case IF_TYPES_SX1276_LORA:
            iface->handler.sx1276_lora->transc.ll_func.switch_to_rx = NULL;
            break;
#endif
#ifdef INTERFACE_SX1276_FSK
        case IF_TYPES_SX1276_FSK:
            iface->handler.sx1276_fsk->transc.ll_func.switch_to_rx = NULL;
            break;
#endif
        default:
            res = -1;
            break;
    }


    return res;
}

