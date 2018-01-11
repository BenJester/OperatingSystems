package edu.duke.raft;

import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;

public class StartClient {
  public static void main (String[] args) {
        if (args.length != 2) {
      System.out.println ("usage: java edu.duke.raft.StartServer -Djava.rmi.server.codebase=<codebase url> <int: rmiregistry port> <int: server id>");
      System.exit(1);
    }
    int port = Integer.parseInt (args[0]);    
    int id = Integer.parseInt (args[1]);
    
    String url = "rmi://localhost:" + port + "/S" + id;
    System.out.println ("Testing S" + id);
    System.out.println ("Contacting server via rmiregistry " + url);

    try {
      RaftServer server = (RaftServer) Naming.lookup(url);
      server.requestVote (0, 0, 0, 0);
      server.appendEntries (0, 0, 0, 0, null, 0);
    } catch (MalformedURLException me) {
      System.out.println (me.getMessage());
    } catch (RemoteException re) {
      System.out.println (re.getMessage());
    } catch (NotBoundException nbe) {
      System.out.println (nbe.getMessage());
    }    
  }
}
