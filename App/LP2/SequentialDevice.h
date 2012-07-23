/**
 * \file SequentialDevice.h
 *
 * Copyright (c) 2011, Topcon Medical Systems, Inc.
 * All rights reserved.
 *
 * Declare the iSequentialDevice interface.
 */

#ifndef SEQUENTIAL_DEVICE_H
#define SEQUENTIAL_DEVICE_H

//******************
// INCLUDES
//******************

#include <QIODevice>

class QSocketNotifier;

//******************
// CLASS
//******************

/**
 * This class defines the implementation of QIODevice to use for
 * sequential character devices.  Examples of these types of devices
 * include serial ports and the DIO ports exposed by the synthesis-dio
 * kernel driver.
 *
 * This class implements all of the QIODevice methods necessary to use
 * these devices with other Qt classes.
 */
class iSequentialDevice : public QIODevice
{
    Q_OBJECT

public:

    /**
     * Initialize the sequential device.  This constructor does NOT
     * open the device.
     *
     * \param path
     *   Path to the file that we will open.
     */
    iSequentialDevice( const QString& path );
    virtual ~iSequentialDevice( void ) = 0;

    /**
     * Determine the number of bytes available for reading.
     *
     * \return
     *   The number of bytes available for reading, between the
     *   QIODevice internal buffer and this class' internal buffer.
     */
    virtual qint64 bytesAvailable( void ) const;

    /**
     * Get the number of bytes waiting to be written.
     *
     * \return
     *   0.  This device never waits to write data.
     */
    virtual qint64 bytesToWrite( void ) const;

    /**
     * Close this device.  Once closed, the device must be re-opened
     * successfully before it may be read from or written to.
     */
    virtual void close( void );

    /**
     * Determine if this device is sequential.  Qt will not allow
     * seek() calls on sequential devices.
     *
     * \return
     *   True.  These devices are always sequential.
     */
    virtual bool isSequential( void ) const;

    /**
     * Open the device for reading and/or writing.  Once this method
     * is called, Qt will allow read and write calls to be made,
     * depending on the open mode.
     *
     * \param mode
     *   Mode to open the file in.  May be ReadOnly, WriteOnly, or
     *   ReadWrite.
     *
     * \return
     *   True if we could open this device in the given mode.  False
     *   if we could not.
     */
    virtual bool open( OpenMode mode );

protected:

    /**
     * Implementation for QIODevice.  This method copies data from our
     * internal buffer into the provided user buffer.
     *
     * \param data
     *     Data buffer to populate.
     * \param maxSize
     *     Maximum number of bytes to place in the buffer.
     *
     * \return
     *     The number of bytes actually copied into the user buffer.
     */
    virtual qint64 readData( char* data, qint64 maxSize );

    /**
     * Implementation for QIODevice.  This method writes data from
     * the provided user buffer to the device.
     *
     * \param data
     *     Data buffer to write from
     * \param maxSize
     *     Maximum number of bytes to write from the buffer.
     *
     * \return
     *     The number of bytes actually written from the user buffer.
     */
    virtual qint64 writeData( const char* data, qint64 maxSize );

    /**
     * Provide special flags that allow subclasses to customize how
     * the device is opened.  These flags are OR'd with the open mode
     * flags.
     *
     * The default implementation provides no extra flags.
     *
     * \return
     *     Extra flags to use when opening the device.
     */
    virtual uint32_t DeviceFlags( void ) const;

    /**
     * Perform extra setup on the opened device.  This method is
     * called after the device is opened.  Subclasses may implement
     * this method to perform specialized setup.
     *
     * The default implementation does nothing, and returns success.
     *
     * \param fd
     *     File descriptor of the open device.
     *
     * \return
     *     0 on success, or -1 on failure.
     */
    virtual int32_t SetupDevice( int fd );

private slots:

    void FileActivated( void );

private:

    QByteArray mDataBuffer;

    QString mFilePath;
    int     mFileDescriptor;

    QSocketNotifier* mpNotifier;
};

#endif
