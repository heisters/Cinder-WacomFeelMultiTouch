#include "WacomFeelMultiTouch.h"
#include "cinder/Log.h"

using namespace wacom;
using namespace ci;

#pragma mark Wacom Touch API C-Function Callbacks

//--------------------------------------------------------------
// Purpose:	A new device was connected.
//
void ciWacomFeelMultiTouchAttachCallback( WacomMTCapability deviceInfo, void *userInfo )
{
	FeelMultiTouch* t = (FeelMultiTouch*)userInfo;
	t->_deviceAttached( deviceInfo );
}



//--------------------------------------------------------------
// Purpose:	A device was unplugged.
//
void ciWacomFeelMultiTouchDetachCallback( int deviceID, void *userInfo )
{
	FeelMultiTouch* t = (FeelMultiTouch*)userInfo;
	t->_deviceDetached( deviceID );
}



//--------------------------------------------------------------
// Purpose:	Fingers are moving on one of the connected devices.
//
int ciWacomFeelMultiTouchFingerCallback( WacomMTFingerCollection *fingerPacket, void *userInfo )
{
	FeelMultiTouch* t = (FeelMultiTouch*)userInfo;
	t->_fingerCallBack( fingerPacket );

	return 0;
}

#pragma mark wacom::FeelMultiTouch member functions

//--------------------------------------------------------------
FeelMultiTouch::FeelMultiTouch() :
	mWidth( 1.f ),
	mHeight( 1.f )
{
	init();
}

FeelMultiTouch::FeelMultiTouch( float w, float h ) :
	mWidth( w ),
	mHeight( h )
{
	init();
}


//--------------------------------------------------------------
FeelMultiTouch::~FeelMultiTouch()
{
}

//--------------------------------------------------------------
bool FeelMultiTouch::init()
{
	bool wasInitialised = false;

	// From the Wacom SDK Example: 
	// The WacomMultiTouch framework is weak-linked. That means the application 
	// can load if the framework is not present. However, we must take care not 
	// to call framework functions if the framework wasn't loaded. 
	//
	// You can set WacomMultiTouch.framework to be weak-linked in your own 
	// project by opening the Info window for your target, going to the General 
	// tab. In the Linked Libraries list, change the Type of 
	// WacomMultiTouch.framework to "Weak". 

	if ( WacomMTInitialize != NULL )
	{
		WacomMTError err = WacomMTInitialize( WACOM_MULTI_TOUCH_API_VERSION );

		if ( err == WMTErrorSuccess )
		{
			CI_LOG_V( "Wacom SDK initialized" );

			// Listen for device connect/disconnect.
			// Note that the attach callback will be called for each connected device 
			// immediately after the callback is registered. 
			WacomMTRegisterAttachCallback( ciWacomFeelMultiTouchAttachCallback, this );
			WacomMTRegisterDetachCallback( ciWacomFeelMultiTouchDetachCallback, this );

			int   deviceIDs[MAX_ATTACHED_DEVICES] = {};
			int   deviceCount = 0;
			int   counter = 0;

			// Add a viewer for each device's data
			deviceCount = WacomMTGetAttachedDeviceIDs( deviceIDs, sizeof( deviceIDs ) );
			if ( deviceCount > MAX_ATTACHED_DEVICES )
			{
				// With a number as big as 30, this will never actually happen.
				CI_LOG_E( "More tablets connected than would fit in the supplied buffer. Will need to reallocate buffer!" );
			}
			else
			{
				for ( counter = 0; counter < deviceCount; counter++ )
				{
					WacomMTRegisterFingerReadCallback( deviceIDs[counter], NULL, WMTProcessingModeNone, ciWacomFeelMultiTouchFingerCallback, this );
				}

				wasInitialised = true;
			}

		}
	}
	else
	{
		// WacomMultiTouch.framework is not installed.
		CI_LOG_E( "unable to initialize the Wacom SDK." );
	}

	return wasInitialised;
}

//--------------------------------------------------------------
void FeelMultiTouch::_deviceAttached( WacomMTCapability deviceInfo )
{
	CI_LOG_V( "tablet device attached" );
}

//--------------------------------------------------------------
void FeelMultiTouch::_deviceDetached( int deviceID )
{
	CI_LOG_V( "tablet device detached" );
}

//--------------------------------------------------------------
void FeelMultiTouch::_fingerCallBack( WacomMTFingerCollection *fingerPacket )
{
	//cout << "fingerPacket->FingerCount: " << fingerPacket->FingerCount << endl;

	double now = app::getElapsedSeconds();

	for ( int fingerIndex = 0; fingerIndex < fingerPacket->FingerCount; fingerIndex++ )
	{
		WacomMTFinger* finger = &fingerPacket->Fingers[fingerIndex];

		// From http://www.wacomeng.com/touch/WacomFeelMulti-TouchAPI.htm
		// Confidence: If true the driver believes this is a valid touch from a finger.  
		// If false the driver thinks this may be an accidental touch, forearm or palm.

		if ( finger->Confidence )
		{
			app::TouchEvent::Touch touch(
				vec2( finger->X, finger->Y ) * vec2( mWidth, mHeight ), // position
				vec2(), // previous position (TODO)
				finger->FingerID, // id (this is actually the index of the finger, not a unique ID)
				now, // time
				finger ); // native event
			
			app::TouchEvent event( app::getWindow(), { touch } );

			if ( finger->TouchState == WMTFingerStateNone )
			{
				//cout << "State None" << endl;
			}
			else if ( finger->TouchState == WMTFingerStateDown )
			{
				mTouchesBeganSignal.emit( event );
			}
			else if ( finger->TouchState == WMTFingerStateHold )
			{
				mTouchesMovedSignal.emit( event );
			}
			else if ( finger->TouchState == WMTFingerStateUp )
			{
				mTouchesEndedSignal.emit( event );
			}
		}
	}
}


//--------------------------------------------------------------
// We scale the normalized input by these coordinates
void FeelMultiTouch::setSize( float _w, float _h )
{
	mWidth = _w;
	mHeight = _h;
}

//--------------------------------------------------------------
std::string FeelMultiTouch::describeAttachedDevices()
{
	int   deviceIDs[MAX_ATTACHED_DEVICES] = {};
	int   deviceCount = 0;
	int   counter = 0;
	std::stringstream out;

	// Ask the Wacom API for all connected touch API-capable devices.
	// Pass a comfortably large buffer so you don't have to call this method 
	// twice. 
	deviceCount = WacomMTGetAttachedDeviceIDs( deviceIDs, sizeof( deviceIDs ) );

	if ( deviceCount > MAX_ATTACHED_DEVICES )
	{
		// With a number as big as MAX_ATTACHED_DEVICES, this will never actually happen.
		CI_LOG_E( "More tablets connected than would fit in the supplied buffer. Will need to reallocate buffer!" );
	}
	else
	{
		for ( counter = 0; counter < deviceCount; counter++ )
		{
			int                  deviceID = deviceIDs[counter];
			WacomMTCapability    capabilities = {};

			WacomMTGetDeviceCapabilities( deviceID, &capabilities );

			out << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << std::endl;
			out << "Device ID: " << deviceID << "  ";
			if ( capabilities.Type == WMTDeviceTypeIntegrated )
			{
				out << "Type: Integrated." << std::endl;
			}
			else if ( capabilities.Type == WMTDeviceTypeOpaque )
			{
				out << "Type: Opaque." << std::endl;
			}
			else
			{
				out << "Type: Unknown." << std::endl;
			}
			out << "Max Fingers: " << capabilities.FingerMax << "  Scan size: " << capabilities.ReportedSizeX << ", " << capabilities.ReportedSizeY << std::endl;
			out << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << std::endl;
		}
	}

	return out.str();
}

