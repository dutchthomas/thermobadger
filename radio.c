
#define PORT_CE PORTD
#define PIN_CE 3

void nrfGet();
char tempProcess(char in);
void enableINT0();
void nrfPrintReg();

void enableINT0()
{
    // Enable input on interupt
	DDRD &= ~(1<<PD2);
	
	// Enable pull-up resistor
	PORTD |= 1<<PD2;
	
	// Enable INT0 mask
	EIMSK |= 1<<INT0;		// Enable INT0
	
	// Falling edge detect
	EICRA = 1<<ISC01;
	
	// Enable interupts
	sei();
}



//Interrupt Service Routine for INT0
ISR(INT0_vect)
{
    // Get a packet
    nrfGet();
    
    // Clear the interupt (AVR)
    EIFR = 1 << INTF0;
}

void statusPrint()
{
    char *msg = "Temp: --  | Target: --  | System: Off | Fan: Auto \r";
    
    if(enable)
    {
        memcpy(msg+34, "On ", 3);
    }
    else
    {
        memcpy(msg+34, "Off", 3);
    }
    
    if(fanMode)
    {
        memcpy(msg+45, "On  ", 4);
    }
    else
    {
        memcpy(msg+45, "Auto", 4);
    }

    
    if(temperature)
    {
        msg[6] = '0' + temperature/10;
        msg[7] = '0' + temperature%10;
        msg[8] = 'F';
    }
    else
    {
        memcpy(msg+6, "-- ", 3);
    }
    
    if(d1 == -1)
    {
        msg[20] = '-';
    }
    else
    {
        msg[20] = '0' + d1;
    }

    if(d0 == -1)
    {
        msg[21] = '-';
    }
    else
    {
        msg[21] = '0' + d0;
    }
        
    if(temp)
    {
        msg[22] = 'F';
    }
    
    uoutSend(msg);
}

void nrfPrintReg()
{
        int i;
        unsigned char data;
        
        for(i = 0; i < 10; i++)
        {
            spiRead( 0x00 | i, &data, 1);
            
            uoutSend("Reg "); uoutSendInt(i, 10); uoutSend(": "); uoutSendInt(data, 16); uoutSend("\n\r");
            
        }
        
        uoutSend("\n\r");
}

void nrfMaster()
{
    unsigned char masterMac[3] = { 0x56, 0x34, 0x12 }; // MAC address of clicker, packed backwards

    // Enable config line
    PORT_CE = SetBit(PORT_CE, PIN_CE, 0);
    
    spiWriteByte(NRF_REG_W | NRF_CONFIG, 0x0A); // Power on
    
    delay_ms(2);
    
    spiWriteByte(NRF_REG_W | NRF_CONFIG, 0x3F); // 2byte CRC, receive mode, IRQ on RX
    spiWriteByte(NRF_REG_W | NRF_EN_RXADDR, 0x01); // enable RX pipe 1
    spiWriteByte(NRF_REG_W | NRF_RX_PW_P0, 4); // packet size = mac length + data length = 3 + 1 = 4
    spiWriteByte(NRF_REG_W | NRF_EN_AA, 0x00); // disable auto ack, clicker does not support it
    spiWriteByte(NRF_REG_W | NRF_RF_CH, NRF_CHANNEL); // Set channel
    spiWriteByte(NRF_REG_W | NRF_SETUP_AW, 0x01); // 3-byte MAC
    spiWriteByte(NRF_REG_W | NRF_RF_SETUP, 0x06); // 1Mbps, high power
    
    spiWrite(NRF_FLUSH_RX, 0, 0); // Flush receive buffer
    
    spiWriteByte(NRF_REG_W | NRF_STATUS, 0x70); // Clear interupt
    spiWrite(NRF_REG_W | NRF_RX_ADDR_P0, masterMac, 3); // Send mac address to listen on

    
    // Disable config line, enable receiver
    PORT_CE = SetBit(PORT_CE, PIN_CE, 1);
}

void nrfAck(unsigned char status, unsigned char *mac, int len)
{
    // Enable config line
    PORT_CE = SetBit(PORT_CE, PIN_CE, 0);

    spiWriteByte(NRF_REG_W | NRF_CONFIG, 0x0A); // Power on
    
    delay_ms(2);

    spiWriteByte(NRF_REG_W | NRF_CONFIG, 0x3E); // 2byte CRC, transmit mode
    spiWriteByte(NRF_REG_W | NRF_EN_AA, 0x00); // disable auto ack, clicker does not support it
    spiWriteByte(NRF_REG_W | NRF_SETUP_AW, 0x01); // 3-byte MAC
    spiWriteByte(NRF_REG_W | NRF_SETUP_RETR, 0x00);// Disable retransmit
    spiWriteByte(NRF_REG_W | NRF_RF_CH, NRF_CHANNEL); // Set channel

    spiWriteByte(NRF_REG_W | NRF_RF_SETUP, 0x06); // 1Mbps, high power
    
    spiWrite(NRF_FLUSH_TX, 0, 0); // Flush transmit buffer
    
    spiWriteByte(NRF_REG_W | NRF_STATUS, 0x70); // Clear IRQ...
    
    spiWriteByte(NRF_W_TX_PAYLOAD, status); // Set packet...

    spiWrite(NRF_REG_W | NRF_TX_ADDR, mac, 3); // Send mac address to transmit to 

    // Disable config line, send packet
    PORT_CE = SetBit(PORT_CE, PIN_CE, 1);
    
    // Keep it high for transmit
    delay_ms(2);
    
    unsigned char nrfStatus;
    
    spiRead(NRF_REG_R | NRF_STATUS, &nrfStatus, 1);
    
    if(GetBit(nrfStatus, 5))
    {
        //uoutSend("TX packet sent \n\r");
    }
    else
    {
        //uoutSend("TX packet NOT sent \n\r");
    }
    
    spiWriteByte(NRF_REG_W | NRF_STATUS, 0x60); // Clear IRQ...
}

void nrfGet()
{
    unsigned char status;
    
    spiRead(NRF_REG_R | NRF_STATUS, &status, 1);
    
    if(!GetBit(status, 6))
    {
        //uoutSend("RX packet NOT ready \n\r");
    }
    else
    {
    
        unsigned char packet[4]; // mac + data = 3 + 1 = 4

        spiRead(NRF_R_RX_PAYLOAD, packet, 4); // Get packet...
        spiWriteByte(NRF_REG_W | NRF_STATUS, 0x70); // Clear IRQ, data rx bit
        
        char result = tempProcess(packet[3]);
        
        unsigned char rmac[3] = {0xC9, 0x87, 0x93};
        
        // ACK the packet
        nrfAck(result, rmac, 4); // packet length 4
        
        // Back to master
        nrfMaster();
       
        /*
        uoutSend("Packet: ");
        
        int i;
        for(i = 0; i < 4; i++)
        {
            uoutSendInt(packet[i], 16);
            uoutSend(" ");
        }
        
        uoutSend("\n\r");
        */
        
    }

}



void tempCancel()
{
    digOn = 0;
    d1 = temp/10;
    d0 = temp % 10;
    remoteTimeout = -1;
}

char tempProcess(char in)
{
    char t = 0;
    
    if(in == 0x3F)
    {
        digMode = 1;
        return 0x06;
    }
    else if(digMode)
    {
        if(in == 0x31)
        {
            enable = !enable;
            if(enable)
            {
                temp = temperature;
                tempCancel();
            }
        }
        else if(in == 0x32)
        {
            fanMode = !fanMode;
        }
        else
        {
            digMode = 0;
            return 0x05;
        }
        
        digMode = 0;
        return 0x06;
    }

    if(in >= 0x30 && in <= 0x39)
    {
        remoteTimeout = 0;
        in &= 0x0F;
        
        if(digOn == 0)
        {
            d1 = in;
            d0 = -1;
            digOn = 1;
        }
        else if(digOn == 1)
        {
            d0 = in;
            
            t = d1*10 + d0;
            
            // Start over...
            digOn = 0;
            
            // Valid
            if(t >= MIN_TEMP && t <= MAX_TEMP)
            {
                //uoutSend("Temp: ");
                //uoutSendInt(t, 10);
                //uoutSend("\n\r");
                
                temp = t;
                remoteTimeout = -1;
                return 0x06;
            }
            // Invalid
            else
            {
                //uoutSend("Invalid Temp.\n\r");
                tempCancel();
                return 0x05;
            }

        }

    }
    
    return 0x06;
}
