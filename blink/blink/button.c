#include "button.h"

void button_init(button_t *btn, uint pin, bool active_low)
{
    btn->pin = pin;
    btn->active_low = active_low;

    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);

    // Use internal pull-up or pull-down depending on active level
    if (active_low)
        gpio_pull_up(pin); // HIGH → not pressed, LOW → pressed
    else
        gpio_pull_down(pin); // LOW → not pressed, HIGH → pressed
}

bool button_is_pressed(button_t *btn)
{
    bool raw = gpio_get(btn->pin);

    // Convert raw state depending on active level
    return btn->active_low ? !raw : raw;
}
