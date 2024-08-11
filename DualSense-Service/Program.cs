using System.Net.Sockets;
using System.Net;
using Wujek_Dualsense_API;
using System.Diagnostics;

IPEndPoint ipendPoint = new IPEndPoint(IPAddress.Any, 0);
UdpClient client = new UdpClient(64370);
Dualsense dualsense = null;

while(dualsense == null)
{
    try
    {
        dualsense = new Dualsense(0);
        dualsense.Connection.ControllerDisconnected += Connection_ControllerDisconnected;
        dualsense.Start();
        break;
    }
    catch
    {
        Thread.Sleep(2500);
        continue;
    }
}

void Connection_ControllerDisconnected(object? sender, ConnectionStatus.Controller e)
{
    dualsense = null;

    while (dualsense == null)
    {
        try
        {
            dualsense = new Dualsense(0);
            dualsense.Connection.ControllerDisconnected += Connection_ControllerDisconnected;
            dualsense.Start();
        }
        catch
        {
            Thread.Sleep(2500);
            continue;
        }
    }
}

float ConvertByteToFloat(byte value)
{
    if (value < 100)
    {
        return value / 100.0f;
    }
    else
    {
        return 1.0f;
    }
}

float speakerVolume = 1f;
float leftActuatorVolume = 1f;
float rightActuatorVolume = 1f;
bool clearBuffer = false;
bool lightbarTransition = false;
Stopwatch cooldown = new Stopwatch();
cooldown.Start();

while (true)
{
    if (dualsense != null && dualsense.Working)
    {
        try
        {
            byte[] bytes = client.Receive(ref ipendPoint);
            if (bytes[0] == 0xFC || bytes[0] == 0xFF)
            {
                //Console.WriteLine((TriggerType.TriggerModes)bytes[9] + " " + bytes[10]);
                //dualsense.SetVibrationType((Vibrations.VibrationType)bytes[0]);
                Console.WriteLine((TriggerType.TriggerModes)bytes[9]);
                dualsense.SetLeftTrigger((TriggerType.TriggerModes)bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[8]);
                dualsense.SetRightTrigger((TriggerType.TriggerModes)bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15], bytes[16]);
                dualsense.SetMicrophoneLED((LED.MicrophoneLED)bytes[17]);
                dualsense.SetPlayerLED((LED.PlayerLED)bytes[18]);
                dualsense.SetStandardRumble(bytes[22], bytes[23]);
                speakerVolume = ConvertByteToFloat(bytes[24]);
                leftActuatorVolume = ConvertByteToFloat(bytes[25]);
                rightActuatorVolume = ConvertByteToFloat(bytes[26]);
                clearBuffer = Convert.ToBoolean(bytes[27]);
                //lightbarTransition = Convert.ToBoolean(bytes[28]);
                dualsense.SetLightbar(bytes[19], bytes[20], bytes[21]);
            }
            else
            {
                string wavFileLocation = System.Text.Encoding.UTF8.GetString(bytes, 0, bytes.Length);
                dualsense.SetVibrationType(Vibrations.VibrationType.Haptic_Feedback);
                Console.WriteLine(wavFileLocation);
                if (File.Exists(wavFileLocation) && cooldown.ElapsedMilliseconds >= 350)
                {
                    dualsense.PlayHaptics(wavFileLocation, speakerVolume, leftActuatorVolume, rightActuatorVolume, true);
                    cooldown.Restart();
                }
            }
        }
        catch (Exception e)
        {
            Console.WriteLine(e.Message);
        }
    }
}
