
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.net.UnknownHostException;
/*from ww  w . jav  a  2  s .co m*/
/**
 * @author lars
 * http://www.java2s.com/Tutorials/Java/Java_Network/0020__Java_Network_TCP_Client_Socket.htm
 */
public class ClientMain {
	public static void main(String[] args) throws Exception {
		String ip = "";
		String cmd = "";
		if (args.length > 0) {
			ip = args[0];
		}
		if (args.length > 1) {
			cmd = args[1];
		}
		if (cmd != null) {
			String status = toggleLed(ip, cmd);
			System.out.println(" Response: " + status);

/*				for (int i=0; i<10; i++){
					System.out.print("Iteration:" + i + "\t");
					status = toggleLed("input");
					System.out.println("   status: " + status);
					Thread.sleep(1000);
				}*/
		}
/*		
		for (int i=0; i<10000; i++){
			System.out.print("Iteration:" + i + "\t");
			String status = toggleLed("state_request");
			System.out.println("   status: " + status);
			Thread.sleep(1000);
		}
		*/
	}

	private static String toggleLed(String serverName, String outMsg) throws UnknownHostException, IOException {
		//	String serverName = "lenovo_windows";
		//	int serverPort = 12900;
		// String serverName = "192.168.1.179";
		// String serverName = "Alarm";
		// String serverName="ENC28JBE0002";
		// String serverName="Alar2";

		int serverPort = 12900;
	    Socket socket = new Socket(serverName, serverPort);
	    socket.setReuseAddress(true);
	    System.out.print(socket.getLocalSocketAddress() + " -> " + socket.getRemoteSocketAddress());
	    BufferedReader socketReader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
	    BufferedWriter socketWriter = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));

	    String inMsg = null;
		// Add a new line to the message to the server,
		// because the server reads one line at a time.
		socketWriter.write(outMsg);
		socketWriter.write("\n");
		socketWriter.flush();
		
		  // Read and display the message from the server
		inMsg = socketReader.readLine();
		//System.out.println("\tFrom server: " + inMsg);
		socket.close();
		return inMsg;
	}
}