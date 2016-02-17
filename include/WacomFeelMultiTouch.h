#pragma once

#include <Wacom_Feel_SDK/WacomMultiTouch.h>
#include "cinder/Signals.h"
#include "cinder/app/App.h"

#define MAX_ATTACHED_DEVICES 30


namespace wacom {
	class FeelMultiTouch
	{

	public:

		typedef ci::signals::Signal< void ( ci::app::TouchEvent ) > signal_t;

		FeelMultiTouch();
		FeelMultiTouch( float w, float h );
		~FeelMultiTouch();

		bool init();
		void setSize( float _w, float _h ); // What do we want the max width and height returned to be?

		static std::string describeAttachedDevices();

		void _deviceAttached( WacomMTCapability deviceInfo );
		void _deviceDetached( int deviceID );

		void _fingerCallBack( WacomMTFingerCollection *fingerPacket );

		
		signal_t& getTouchesBeganSignal() { return mTouchesBeganSignal; }
		signal_t& getTouchesMovedSignal() { return mTouchesMovedSignal; }
		signal_t& getTouchesEndedSignal() { return mTouchesEndedSignal; }
	private:

		float mWidth, mHeight;

		signal_t mTouchesBeganSignal;
		signal_t mTouchesMovedSignal;
		signal_t mTouchesEndedSignal;
	};
}