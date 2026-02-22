import pyttsx3

engine = pyttsx3.init()
voices = engine.getProperty('voices')

print("--- AVAILABLE VOICES ---")
found_arabic = False

for voice in voices:
    print(f"Name: {voice.name}")
    print(f"ID: {voice.id}")
    if "Arabic" in voice.name or "Hoda" in voice.name or "Naayf" in voice.name:
        found_arabic = True
        print("✅ ARABIC DETECTED HERE!")
    print("------------------------")

if found_arabic:
    print("\n🎉 Success! Arabic voice is ready.")
else:
    print("\n❌ Not found yet. Try restarting your computer.")