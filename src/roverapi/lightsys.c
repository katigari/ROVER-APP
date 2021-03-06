
 /*
 * 
 * Copyright (c) 2016 Eclipse Foundation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Authors:
 *    Mobin Pourreza; mobin.pourreza@gmail.com
 * Contributors:
 */
 

#include <roverapi/lightsys.h>


// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                19				/// select the GPIO pin to send data. 
#define DMA                     10				/// select DMA channel to use 
#define STRIP_TYPE              WS2811_STRIP_GBR		// WS2812/SK6812RGB integrated chip+leds


#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

#define WIDTH                   4	/// number of lights
#define HEIGHT                  1	/// in case of light matrix: number of lights rows
#define LED_COUNT               (WIDTH * HEIGHT)

int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

static uint8_t blink = 0;		// Key to stop the lighting loop

ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = 0,//GPIO_PIN,
            .count = 0,//LED_COUNT,
            .invert = 0,
            .brightness = 0,//255,
	    .strip_type = STRIP_TYPE,
        },
        [1] =
        {
            .gpionum = GPIO_PIN, //.gpionum = 0,
            .count = LED_COUNT, //.count = 0,
            .invert = 0,
            .brightness = 255, //0,
	    .strip_type = STRIP_TYPE, // this line shall be deleted
        },
    },
};

ws2811_led_t *matrix;

/// Define colors
ws2811_led_t	blue	= 0xf00000;
ws2811_led_t	red 	= 0x00f000;
ws2811_led_t	red_lite= 0x002000;
ws2811_led_t	green 	= 0x0000f0;
ws2811_led_t	white 	= 0xf0f0f0;
ws2811_led_t	yellow	= 0x00f090;
ws2811_led_t	orange 	= 0x00a020;
ws2811_led_t	purple 	= 0x50a000;
ws2811_led_t	pink 	= 0x20c020;
ws2811_led_t	off 	= 0x000000;
/// define temps for saving colors
ws2811_led_t	colortemp1= 0;
ws2811_led_t	colortemp2= 0;

int clear_on_exit = 0;

/// define each light position regarding its position in series connection
#define	Light_F_R	matrix[0] 	///Front Right light
#define	Light_F_L	matrix[1]	///Front Left light
#define	Light_R_R	matrix[3]	///Rear Right light
#define	Light_R_L	matrix[2]	///Rear Left light


/// assign the matrix values to final  light data matrix
void matrix_render(void)
{
    int x, y;

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            ledstring.channel[1].leds[(y * width) + x] = matrix[y * width + x];
        }
    }
}


void matrix_clear(void)
{
    int x, y;

    for (y = 0; y < (height ); y++)
    {
        for (x = 0; x < width; x++)
        {
            matrix[y * width + x] = 0;
        }
    }
}


/// to stop lighting loops by keyboard
static void ctrl_c_handler(int signum)
{
	(void)(signum);
    blink = 0;
}

void stop_blink()
{	
    blink = 0;
}

void setup_handlers(void)//static void setup_handlers(void)
{
    struct sigaction sa =
    {
        .sa_handler = ctrl_c_handler,
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
} 



/**
 * Allocate and initialize memory, buffers, pages, PWM, DMA, and GPIO.
 *
 * @returns  0 on success, -1 otherwise.
 */

ws2811_return_t initial(void)
{
    ws2811_return_t ret;
    matrix = malloc(sizeof(ws2811_led_t) * width * height);
    setup_handlers();

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
	return ret;
    }
}

/**
 * turn on the lights.
 *
 * @returns  0 on success, -1 otherwise.
 */
void light_on(void)
{
    ws2811_return_t ret;
       
	Light_F_R = white;
	Light_F_L = white;
	Light_R_R = red_lite;
	Light_R_L = red_lite;
	matrix_render();
	
        if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render_light_on failed: %s\n", ws2811_get_return_t_str(ret));
           
        }
            
}


/**
 * Dim the lights.
 *
 * @param   int dim, a value between 15 and 255.
 *
 * @returns  0 on success, -1 otherwise.
 */

void light_dim(int dim)
{
	ws2811_return_t ret;
	
	
	ws2811_led_t	Led_B	= dim << 16;
	fprintf(stderr, "ws2811_render_light_on failed: %X\n", Led_B);
	ws2811_led_t	Led_R	= dim << 8;
	fprintf(stderr, "ws2811_render_light_on failed: %X\n", Led_R);
	ws2811_led_t	Led_G	= dim;
	fprintf(stderr, "ws2811_render_light_on failed: %X\n", Led_G);
	ws2811_led_t	Led_dim = Led_B | Led_R | Led_G;
	fprintf(stderr, "ws2811_render_light_on failed: %X\n", Led_dim);
	ws2811_led_t	red_dim= dim << 8;
	Light_F_R = Led_dim;
	Light_F_L = Led_dim;
	Light_R_R = red_dim;
	Light_R_L = red_dim;
	matrix_render();
	
        if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render_light_on failed: %s\n", ws2811_get_return_t_str(ret));
           
        }
            
}

/**
 * turn off the lights.
 *
 * @returns  0 on success, -1 otherwise.
 */
void light_off(void)
{
    ws2811_return_t ret;
       
	Light_F_R = off;
	Light_F_L = off;
	Light_R_R = off;
	Light_R_L = off;
	matrix_render();
	
        if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render_light_on failed: %s\n", ws2811_get_return_t_str(ret));
           
        }

        
    
}

/**
 * Turn on backward lights in yellow.
 *
 * @returns  0 on success, -1 otherwise.
 */
void light_BackW (void)
{
    ws2811_return_t ret;
   
	colortemp1 = Light_R_R;
	colortemp2 = Light_R_L;       
	
	Light_R_R = yellow;
	Light_R_L = yellow;
	matrix_render();
	
        if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render_light_on failed: %s\n", ws2811_get_return_t_str(ret));
           
        }
    
}


/**
 * Blink the right lights in orange as right indicator.
 *
 * @returns  0 on success, -1 otherwise.
 */

void light_Blink_R(void)
{
    
    ws2811_return_t ret;

    
    // save the last colors before blinking
    colortemp1 = Light_F_R;
    colortemp2 = Light_R_R;
    
	Light_F_R = orange;
	Light_R_R = orange;
	
	matrix_render();
	
	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        }
	usleep(1000000 / 4);
		
	Light_F_R = colortemp1;
	//Light_F_L = off;
	Light_R_R = colortemp2;
	//Light_R_L = off;
	matrix_render();
	
	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        }
	usleep(1000000 / 4);    
    
}


/**
 * Blink the left lights in orange as indicator.
 *
 * @returns  0 on success, -1 otherwise.
 */
void light_Blink_L(void)
{
    
    ws2811_return_t ret;
    
    // save the last colors in temps
    colortemp1 = Light_F_L;
    colortemp2 = Light_R_L;
      
    blink = 1;

	Light_F_L = orange;
	Light_R_L = orange;
	
	matrix_render();
	
	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        }
	usleep(1000000 / 4);
		
	Light_F_L = colortemp1;
	Light_R_L = colortemp2;
	
	matrix_render();
	
	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        }
	usleep(1000000 / 4); 
    
    
}


/**
 * left blink function without delay.
 *
 * @returns  0 on success, -1 otherwise.
 */

void light_Blink_L_on(void)
{
    
    ws2811_return_t ret;
    
    colortemp1 = Light_F_L;
    colortemp2 = Light_R_L;

	Light_F_L = orange;
	Light_R_L = orange;
	Light_F_R = off;

	Light_R_R = off;
	
	
	matrix_render();
	
	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        }

    
}


/**
 * right blink function without delay
 *
 * @returns  0 on success, -1 otherwise.
 */

void light_Blink_R_on(void)
{
    
    ws2811_return_t ret;
    
    colortemp1 = Light_F_R;
    colortemp2 = Light_R_R;
    
	Light_F_R = orange;
	Light_R_R = orange;

	Light_F_L = off;

	Light_R_L = off;
	
	matrix_render();
	
	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        }

    
}


