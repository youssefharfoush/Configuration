/*
 * helloworld.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: apple
 */



#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>




int str2uuid( const char *uuid_str, uuid_t *uuid )
{
    uint32_t uuid_int[4];
    char *endptr;

    if( strlen( uuid_str ) == 36 ) {
        // Parse uuid128 standard format: 12345678-9012-3456-7890-123456789012
        char buf[9] = { 0 };

        if( uuid_str[8] != '-' && uuid_str[13] != '-' &&
            uuid_str[18] != '-'  && uuid_str[23] != '-' ) {
            return 0;
        }
        // first 8-bytes
        strncpy(buf, uuid_str, 8);
        uuid_int[0] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        // second 8-bytes
        strncpy(buf, uuid_str+9, 4);
        strncpy(buf+4, uuid_str+14, 4);
        uuid_int[1] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        // third 8-bytes
        strncpy(buf, uuid_str+19, 4);
        strncpy(buf+4, uuid_str+24, 4);
        uuid_int[2] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        // fourth 8-bytes
        strncpy(buf, uuid_str+28, 8);
        uuid_int[3] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        if( uuid != NULL ) sdp_uuid128_create( uuid, uuid_int );
    } else if ( strlen( uuid_str ) == 8 ) {
        // 32-bit reserved UUID
        uint32_t i = strtoul( uuid_str, &endptr, 16 );
        if( endptr != uuid_str + 8 ) return 0;
        if( uuid != NULL ) sdp_uuid32_create( uuid, i );
    } else if( strlen( uuid_str ) == 4 ) {
        // 16-bit reserved UUID
        int i = strtol( uuid_str, &endptr, 16 );
        if( endptr != uuid_str + 4 ) return 0;
        if( uuid != NULL ) sdp_uuid16_create( uuid, i );
    } else {
        return 0;
    }

    return 1;
}

int main(void) {
	int i, j, err, sock, dev_id = -1;
	struct hci_dev_info dev_info;
	inquiry_info *info = NULL;
	int num_rsp, length, flags;
	bdaddr_t ba;
	char addr[19] = { 0 };
	char name[248] = { 0 };
	uuid_t uuid = { 0 };
	//Change this to your apps UUID
	//char *uuid_str="66841278-c3d1-11df-ab31-001de000a901";
//	char *uuid_str="0000110c-0000-1000-8000-00805f9b34fb";
	char *uuid_str="66841278-c3d1-11df-ab31-001de000a901";
	uint32_t range = 0x0000ffff;
	sdp_list_t *response_list = NULL, *search_list, *attrid_list;
	int s, loco_channel = -1, status;
	struct sockaddr_rc loc_addr = { 0 };

	(void) signal(SIGINT, SIG_DFL);

	dev_id = hci_get_route(NULL);
	if (dev_id < 0) {
		perror("No Bluetooth Adapter Available");
		exit(1);
	}

	if (hci_devinfo(dev_id, &dev_info) < 0) {
		perror("Can't get device info");
		exit(1);
	}



	sock = hci_open_dev( dev_id );
	if (sock < 0) {
		perror("HCI device open failed");
		free(info);
		exit(1);
	}


	if( !str2uuid( uuid_str, &uuid ) ) {
		perror("Invalid UUID");
		free(info);
		exit(1);
  }

	do {
		printf("Scanning ...\n");
		info = NULL;
		num_rsp = 0;
		flags = 0;
		length = 8; /* ~10 seconds */
		num_rsp = hci_inquiry(dev_id, length, num_rsp, NULL, &info, flags);
		if (num_rsp < 0) {
			perror("Inquiry failed");
			exit(1);
		}




		printf("No of resp %d\n",num_rsp);

		for (i = 0; i < num_rsp; i++) {
			sdp_session_t *session;
			int retries;
			int foundit, responses;
			ba2str(&(info+i)->bdaddr, addr);

char samsungS4[2] = {'E','4'};

if(addr[0] == samsungS4[0] && addr[1] == samsungS4[1]){


			memset(name, 0, sizeof(name));
			if (hci_read_remote_name(sock, &(info+i)->bdaddr, sizeof(name),
					name, 0) < 0)
			strcpy(name, "[unknown]");
			printf("\nFound %s  %s, searching for the the desired service on it now\n\n", addr, name);
			// connect to the SDP server running on the remote machine
sdpconnect:
			session = 0; retries = 0;

			while(!session) {
				bdaddr_t tmp = {0};

				session = sdp_connect( &tmp, &(info+i)->bdaddr, SDP_RETRY_IF_BUSY );
				if(session) break;
				if(errno == EALREADY && retries < 5) {
					perror("Retrying");
					retries++;
					sleep(1);
					continue;
				}
				break;
			}
			if ( session == NULL ) {
				perror("Can't open session with the device");
				free(info);
				continue;
			}
			search_list = sdp_list_append( 0, &uuid );
			attrid_list = sdp_list_append( 0, &range );
			err = 0;
			err = sdp_service_search_attr_req( session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
			sdp_list_t *r = response_list;
			sdp_record_t *rec;

			//printf("response list: %d\n",*r);
			// go through each of the service records
			foundit = 0;
			responses = 0;
			for (; r; r = r->next ) {
					responses++;
					rec = (sdp_record_t*) r->data;
					//printf("rec %d\n",rec);

					sdp_list_t *proto_list;

					// get a list of the protocol sequences
					if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
						//printf("In the protocol list\n");
					sdp_list_t *p = proto_list;
					//printf("Protocol list %d\n",*p);

						// go through each protocol sequence
						for( ; p ; p = p->next ) {
							//printf("Going through the protocol sequence\n");
								sdp_list_t *pds = (sdp_list_t*)p->data;


								// go through each protocol list of the protocol sequence
								for( ; pds ; pds = pds->next ) {
									//printf("Pds %d\n",pds);
									//printf("Go through each protocol list of the protocol sequence\n");

										// check the protocol attributes
										sdp_data_t *d = (sdp_data_t*)pds->data;
										int proto = 0;
										//printf("pds->data %d\n",*d);
										for( ; d; d = d->next ) {
											//printf("d->dtd %d\n",d->dtd);
												switch( d->dtd ) {
														case SDP_UUID16:
														case SDP_UUID32:
														case SDP_UUID128:

																proto = sdp_uuid_to_proto( &d->val.uuid );
																//printf("In SDP_UUID128 and proto= %d\n",proto );
																break;
														case SDP_UINT8:
																if( proto == RFCOMM_UUID ) {
																		//printf("proto: %d\n",proto);
																		//printf("rfcomm channel: %d\n",d->val.int8);
																		loco_channel = d->val.int8;
																		foundit = 1;
																}
																break;
												}
										}
								}
								sdp_list_free( (sdp_list_t*)p->data, 0 );
						}
						sdp_list_free( proto_list, 0 );

					}
					if (loco_channel > 0)
						break;

			}
			printf("No of Responses %d\n", responses);
			printf("Loco Channel %d\n", loco_channel);
			//printf("Found it? (0=No - 1=Yes) %d\n\n\n", foundit);
			if ( loco_channel > 0 && foundit == 1 ) {
				printf("Found service on this device, now gonna blast it with dummy data\n");
				s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
				loc_addr.rc_family = AF_BLUETOOTH;
				loc_addr.rc_channel = loco_channel;
				loc_addr.rc_bdaddr = *(&(info+i)->bdaddr);
				status = connect(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
				if( status < 0 ) {
					perror("uh oh");
				}

				int iterations=1;
				char data[24] = {"Data: "};
				char dig ;

				do {
					dig = (char)(((int)'0')+iterations);
					data[6]=dig;
					status = write(s, data, 24);
					printf ("Wrote %d bytes\n", status);
					printf(data);

					sleep(1);

					iterations++;
					if(iterations>9){iterations=0;}


				} while (status > 0);
				close(s);
				sdp_record_free( rec );
			}

			sdp_close(session);
			if (loco_channel > 0) {
				goto sdpconnect;
				//break;
			}
		}

	}	//For the extra if
	} while (1);

	printf("Exiting...\n");
}



/*
#include <mraa.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <string>
#include <iostream>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>
#include <stdint.h>
#include <spi.h>

using namespace std;






int main()
{


	//Bluetooth
    struct sockaddr_rc addr = { 0 };
    int s, status;
    char dest[18] = "E4:32:CB:53:47:A3";

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    // send a message
    if( status == 0 ) {
        status = write(s, "hello!", 6);
    }

    if( status < 0 ) perror("Uh oh");

    close(s);
    //Bluetooth DONE





		//create a GPIO structure/context
		mraa_gpio_context gpio;

		// Initialize pin 2
	    gpio = mraa_gpio_init(2);

	    // Set direction to OUTPUT
	    mraa_gpio_dir(gpio, MRAA_GPIO_OUT);

	    //To store the LED state, starting true (HIGH)
	    bool ledState=true;


	    //create a second GPIO structure/context
		mraa_gpio_context gpio_in;

	    // Initialize pin 3
	    gpio_in = mraa_gpio_init(3);

	 	// Set direction to INPUT
	    mraa_gpio_dir(gpio_in, MRAA_GPIO_IN);

	    //Initialize read variable for pin 3
	    int digitalInput3 = 0;

	    int i = 0;

	    while(true){

	    	//Read the input at pin 3
	    	//NOTE: At the first iteration the function always returns 1
	    	 digitalInput3 = mraa_gpio_read(gpio_in);

	    	 if(i==0) cout<<"Digital Input pin 3 is initializing"<<endl;
	    	 else{
	    	//Display it on the console
	    	cout<<"Digital Input pin 3: "<<digitalInput3<<endl;
	    	 }

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

	          i++;
	    }


  return 0;
}
*/
