Cinder-WacomFeelMultiTouch
==========================

Cinder interface to the Wacom Feel Multi Touch SDK. Based on [ofxWacomFeelMultiTouch](https://github.com/andreasmuller/ofxWacomFeelMultiTouch).

To use this, you will need to register with Wacom as a developer and download the Feel MultiTouch SDK from [Wacom](https://developer.wacom.com). Then add the SDK to your include path.

Example
-------

```c++
wacom::FeelMultiTouch fmt;
fmt.getTouchesBeganSignal().connect( bind( &MyApp::touchesBegan, this, placeholders::_1 ) );
fmt.getTouchesMovedSignal().connect( bind( &MyApp::touchesMoved, this, placeholders::_1 ) );
fmt.getTouchesEndedSignal().connect( bind( &MyApp::touchesEnded, this, placeholders::_1 ) );
```
