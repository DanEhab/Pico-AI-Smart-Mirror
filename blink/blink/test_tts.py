"""
Quick test to verify pyttsx3 TTS is working
"""

import pyttsx3

print("Testing Windows Text-to-Speech...")
print("=" * 50)

try:
    engine = pyttsx3.init()
    
    # Get available voices
    voices = engine.getProperty('voices')
    print(f"\n✅ TTS Engine initialized successfully!")
    print(f"📢 Available voices: {len(voices)}")
    
    for i, voice in enumerate(voices):
        print(f"   {i}: {voice.name}")
    
    # Configure TTS
    engine.setProperty('rate', 150)
    engine.setProperty('volume', 1.0)
    
    # Use female voice if available
    if len(voices) > 1:
        engine.setProperty('voice', voices[1].id)
        print(f"\n🎤 Using voice: {voices[1].name}")
    else:
        print(f"\n🎤 Using voice: {voices[0].name}")
    
    # Test speech
    print("\n🔊 Speaking test message...")
    test_message = "Welcome to Station 6. Text to speech is working perfectly!"
    print(f"   Message: '{test_message}'")
    
    engine.say(test_message)
    engine.runAndWait()
    
    print("\n✅ Test completed successfully!")
    print("   Your TTS system is ready for the Pico!")
    
except Exception as e:
    print(f"\n❌ Error: {e}")
    print("\n💡 Try reinstalling: pip install --force-reinstall pyttsx3")
