import xml.etree.ElementTree as ET
import pandas as pd

def parse_flow_monitor_to_table(filename):
    tree = ET.parse(filename)
    root = tree.getroot()

    data = []

    flowStats = root.find(".//FlowStats")

    ipv4 = root.find(".//Ipv4FlowClassifier")
    
    for flow in flowStats.findall(".//Flow"):
        flow_id = flow.attrib['flowId']
        tx_packets = int(flow.attrib['txPackets'])
        rx_packets = int(flow.attrib['rxPackets'])
        lost_packets = tx_packets - rx_packets
        delay_sum = float(flow.attrib['delaySum'].split(' ')[0].replace('ns', ''))
        tx_bytes = int(flow.attrib['txBytes'])
        rx_bytes = int(flow.attrib['rxBytes'])
        time_first_tx_packet = float(flow.attrib['timeFirstTxPacket'].split(' ')[0].replace('ns', ''))
        time_last_rx_packet = float(flow.attrib['timeLastRxPacket'].split(' ')[0].replace('ns', ''))
        duration = time_last_rx_packet - time_first_tx_packet
        
        rtt = (delay_sum / rx_packets) * 1e-6 if rx_packets > 0 else 0
        throughput = (rx_bytes * 8 * 1e9) / duration if duration > 0 else 0

        data.append({
            "Flow ID": flow_id,
            "Tx Packets": tx_packets,
            "Rx Packets": rx_packets,
            "Lost Packets": lost_packets,
            "RTT (ms)": rtt,
            "Throughput (bps)": throughput
        })

    for flow in ipv4.findall(".//Flow"):
        source_ip = flow.attrib['sourceAddress']
        destination_ip = flow.attrib['destinationAddress']
        for d in data:
            if d["Flow ID"] == flow.attrib['flowId']:
                d["Source IP"] = source_ip
                d["Destination IP"] = destination_ip
        
    df = pd.DataFrame(data)
    return df

if __name__ == "__main__":
    filename = "/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/second-assignment-spectrum-1ap.xml"
    df = parse_flow_monitor_to_table(filename)
    print(df)
    df.to_latex('/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/spect-1ap.tex', index=False)

    filename = "/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/second-assignment-spectrum-2ap.xml"
    df = parse_flow_monitor_to_table(filename)
    print(df)
    df.to_latex('/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/spect-2ap.tex', index=False)

    filename = "/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/second-assignment-yans-1ap.xml"
    df = parse_flow_monitor_to_table(filename)
    print(df)
    df.to_latex('/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/yans-1ap.tex', index=False)

    filename = "/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/second-assignment-yans-2ap.xml"
    df = parse_flow_monitor_to_table(filename)
    print(df)
    df.to_latex('/home/matteo/ns-3-dev/scratch/ns-3-second-assignment/yans-2ap.tex', index=False)

