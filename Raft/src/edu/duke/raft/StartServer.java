package edu.duke.raft;

import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.RemoteException;

public class StartServer {
  public static void main (String[] args) {
    if (args.length != 4) {
      System.out.println ("usage: java edu.duke.raft.StartServer -Djava.rmi.server.codebase=<codebase url> <int: rmiregistry port> <int: server id> <log file dir> <config file dir>");
      System.exit(1);
    }
    int port = Integer.parseInt (args[0]);    
    int id = Integer.parseInt (args[1]);
    String logPath = args[2] + "/" + id + ".log";
    String configPath = args[3] + "/" + id + ".config";
    
    String url = "rmi://localhost:" + port + "/S" + id;
//    System.out.println ("Starting S" + id);
//    System.out.println ("Binding server on rmiregistry " + url);

    RaftConfig config = new RaftConfig (configPath);
    RaftLog log = new RaftLog (logPath);
    int lastApplied = log.getLastIndex ();
    RaftResponses.init (config.getNumServers (), log.getLastTerm ());

    try {
      RaftMode.initializeServer (config,
				 log,
				 lastApplied, 
				 port, 
				 id);
      RaftServerImpl server = new RaftServerImpl (id);
      RaftServerImpl.setMode (new FollowerMode ());
      
      Naming.rebind(url, server);
    } catch (MalformedURLException me) {
      System.out.println ("S" + id + ": " + me.getMessage());
      me.printStackTrace ();
    } catch (RemoteException re) {
      System.out.println ("S" + id +  ": " + re.getMessage());
      re.printStackTrace ();
    } catch (Exception e) {
      System.out.println ("S" + id +  ": " +  e.getMessage());
      e.printStackTrace ();
    }
  }  
}

  
