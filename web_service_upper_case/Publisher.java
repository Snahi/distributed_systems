import services.UpperCaseService;
import javax.xml.ws.Endpoint;

public class Publisher {



    public static void main(String[] args)
    {
        String url = UpperCaseService.getUrl();
        Endpoint.publish(url, new UpperCaseService());

        System.out.println("UpperCaserService published - " + url);
    }
}
