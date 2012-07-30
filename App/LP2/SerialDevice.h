// File: SerialDevice.h

#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

//******************
// INCLUDES
//******************

#include "SequentialDevice.h"

class QSocketNotifier;


//******************
// CLASS
//******************

/**
 * Provide a concrete implementation of an iSequentialDevice.  This
 * implementation may add methods specific to serial devices, such as
 * setting the baud rate.
 */
class cSerialDevice : public iSequentialDevice
{
    Q_OBJECT

public:

    /**
     * Initialize the serial device.  This constructor does NOT open
     * the device.
     *
     * \param path
     *   Path to the file that we will open.
     */
    cSerialDevice( const QString& path );
    virtual ~cSerialDevice( void );

protected:

    virtual uint32_t DeviceFlags( void ) const;
    virtual int32_t SetupDevice( int fd );
};

#endif
