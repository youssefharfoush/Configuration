/*
 * helloworld.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: apple
 */
#include <mraa.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

using namespace std;
int main()
{
		//create a GPIO structure/context
		mraa_gpio_context gpio;

		// Initialize pin 2
	    gpio = mraa_gpio_init(2);

	    // Set direction to OUTPUT
	    mraa_gpio_dir(gpio, MRAA_GPIO_OUT);

	    //To store the LED state, starting true (HIGH)
	    bool ledState=true;



	    while(true){

	    	// Writes into GPIO accordingly to the state true = 1, false = 0
	        mraa_gpio_write(gpio, ledState?1:0);

	        // changes state by negating
	        ledState=!ledState;

	        //LED State
	          if(ledState){
	        	  cout<<"ON"<<endl;
	          }
	          else{
	        	  cout<<"OFF"<<endl;
	          }

	          //waits one second
	          sleep(1);

	    }


  return 0;
}
