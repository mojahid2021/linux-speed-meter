# Internet Speed Test Feature - User Guide

## Overview

The Internet Speed Meter now includes a comprehensive **Internet Speed Test** feature that allows you to measure your actual internet connection speed, latency, and jitter using real server-based tests.

## What's the Difference?

### Network Monitor (Original Feature)
- **Passive monitoring**: Tracks data flowing through your network interface
- **Real-time stats**: Shows your current download/upload speeds based on actual usage
- **Session tracking**: Records total data transferred during your session
- **Always running**: Continuously monitors your network activity

### Speed Test (New Feature)
- **Active testing**: Performs deliberate download/upload tests to dedicated servers
- **Connection capability**: Measures your maximum internet speed capability
- **Latency & Jitter**: Tests connection quality with ping measurements
- **On-demand**: Run tests whenever you want to check your connection

## How to Use

### Accessing the Speed Test

#### GTK Version (Linux)
1. Click the system tray icon
2. Select "Show Dashboard"
3. Click the **"Speed Test"** tab

#### Qt Version (Windows/Cross-platform)
1. Right-click the system tray icon
2. Select "Show Dashboard"
3. Click the **"Speed Test"** tab

### Running a Test

1. **Select a Test Server**
   - Use the dropdown menu to choose from available servers
   - Cloudflare, Fast.com (Netflix), and LibreSpeed servers are available
   - The closest server is usually selected by default

2. **Start the Test**
   - Click the **"Start Test"** button
   - The test will run automatically through three phases:
     - **Latency Test** (5 pings to measure connection quality)
     - **Download Test** (10 seconds, 4 parallel connections)
     - **Upload Test** (10 seconds, 4 parallel connections)

3. **Monitor Progress**
   - Watch the progress bar advance through each phase
   - See real-time speed during download/upload phases
   - Status messages keep you informed

4. **View Results**
   - **Download Speed**: Your maximum download capability in Mbps
   - **Upload Speed**: Your maximum upload capability in Mbps
   - **Ping**: Average latency to the server in milliseconds
   - **Jitter**: Variation in latency (lower is better)

### Stopping a Test

- Click the **"Stop Test"** button at any time
- The test will halt immediately
- Partial results will be discarded

## Understanding Results

### Download/Upload Speed
- **Measured in Mbps** (Megabits per second)
- **Higher is better**
- Typical ranges:
  - DSL: 5-25 Mbps
  - Cable: 50-300 Mbps
  - Fiber: 100-1000+ Mbps

### Ping (Latency)
- **Measured in ms** (milliseconds)
- **Lower is better**
- Typical ranges:
  - Excellent: < 20 ms
  - Good: 20-50 ms
  - Fair: 50-100 ms
  - Poor: > 100 ms

### Jitter
- **Measured in ms** (milliseconds)
- **Lower is better**
- Indicates connection stability
- Important for gaming, video calls, and VoIP
- Typical ranges:
  - Excellent: < 5 ms
  - Good: 5-15 ms
  - Fair: 15-30 ms
  - Poor: > 30 ms

## Technical Details

### Test Methodology

#### Download Test
- Opens 4 parallel HTTP connections to test server
- Downloads data for 10 seconds total
- First 2 seconds are warmup (TCP slow-start compensation)
- Speed calculated from remaining 8 seconds
- Uses high-resolution timers for accuracy

#### Upload Test
- Opens 4 parallel HTTP POST connections
- Uploads random data for 10 seconds total
- First 2 seconds are warmup
- Server discards uploaded data
- Speed calculated from remaining 8 seconds

#### Ping Test
- Sends 5 HTTP HEAD requests to server
- Measures round-trip time for each
- Calculates min/max/average latency
- Jitter computed from latency variations

### Test Servers

#### Cloudflare
- **Provider**: Cloudflare CDN
- **Global**: Servers worldwide
- **Recommended for**: General testing

#### Fast.com (Netflix)
- **Provider**: Netflix CDN
- **Global**: Servers worldwide
- **Recommended for**: Streaming quality testing

#### LibreSpeed
- **Provider**: Open source speed test servers
- **Global**: Community-hosted servers
- **Recommended for**: Alternative testing

## Tips for Accurate Results

1. **Close background applications**
   - Stop downloads, streaming, or updates
   - Pause cloud sync services
   - Close other bandwidth-heavy apps

2. **Use wired connection when possible**
   - Wi-Fi can limit speeds
   - Ethernet provides more consistent results

3. **Test multiple times**
   - Run 2-3 tests for average results
   - Different times of day may vary

4. **Try different servers**
   - Server location affects results
   - Test with multiple servers for comparison

5. **Check with your ISP**
   - Compare results with your plan speed
   - Contact ISP if significantly slower

## Troubleshooting

### "Test Failed" or Error Messages

**Possible causes:**
- No internet connection
- Firewall blocking test
- Server temporarily unavailable
- DNS resolution issues

**Solutions:**
- Check internet connection
- Try a different test server
- Disable VPN temporarily
- Check firewall settings

### Results Much Lower Than Expected

**Possible causes:**
- Background applications using bandwidth
- Wi-Fi signal issues
- ISP throttling
- Network congestion

**Solutions:**
- Close all other applications
- Move closer to Wi-Fi router or use Ethernet
- Test at different times of day
- Contact your ISP

### Test Takes Too Long or Hangs

**Possible causes:**
- Very slow connection
- Server timeout
- Network instability

**Solutions:**
- Click "Stop Test" and try again
- Try a different server
- Check your network connection

## Privacy & Data

- **No personal data collected**: Tests are anonymous
- **Data not stored on servers**: Uploaded data is immediately discarded
- **Local results only**: Results stored only on your device
- **No tracking**: No analytics or user tracking

## Advanced Usage

### Command Line Testing
The speed test can also be integrated into scripts for automated testing (feature planned for future release).

### Scheduled Tests
Automatic periodic testing can be configured (feature planned for future release).

## Comparison with Network Monitor

| Feature | Network Monitor | Speed Test |
|---------|----------------|------------|
| **Type** | Passive | Active |
| **Purpose** | Track actual usage | Measure capability |
| **When to use** | Monitor daily usage | Check connection quality |
| **Data source** | Your actual traffic | Test server traffic |
| **Accuracy** | Depends on usage | Standardized test |
| **Speed shown** | Current usage | Maximum capability |

## Frequently Asked Questions

**Q: Why is my speed test result different from my actual download speeds?**
A: Speed tests measure your maximum connection capability. Actual download speeds depend on the server you're downloading from, which may be slower than test servers.

**Q: Should I use Network Monitor or Speed Test?**
A: Use both! Network Monitor shows your actual usage patterns. Speed Test verifies your connection is performing as expected.

**Q: How often should I run speed tests?**
A: Test when you notice slow performance, after changing ISP plans, or troubleshooting connectivity issues. No need to test constantly.

**Q: Why do results vary between tests?**
A: Network conditions change constantly. Server load, routing, and local network activity all affect results. Multiple tests give better average.

**Q: Is the speed test accurate?**
A: Yes, the test uses industry-standard methodology similar to Speedtest.net and Fast.com. Results should be comparable.

## Feedback & Support

For issues or suggestions about the speed test feature:
- Open an issue on GitHub
- Include your OS, network type, and error messages
- Describe steps to reproduce any problems
