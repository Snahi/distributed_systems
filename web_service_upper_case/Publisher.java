import services.UpperCaseService;
import javax.xml.ws.Endpoint;

public class Publisher {



    public static void main(String[] args)
    {
        int port = UpperCaseService.PORT;
        if (args.length > 0)
        {
            String portStr = args[0];
            try
            {
                port = Integer.parseInt(portStr);
            }
            catch (Exception e)
            {
                System.out.println("Incorrect port number");
            }
        }

        String url = String.format(UpperCaseService.URL_FORMAT, port);
        Endpoint.publish(url, new UpperCaseService());

        System.out.println("UpperCaserService published - " + url);
    }
}
