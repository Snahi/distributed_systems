
package upper_case_client;

import java.net.MalformedURLException;
import java.net.URL;
import javax.xml.namespace.QName;
import javax.xml.ws.Service;
import javax.xml.ws.WebEndpoint;
import javax.xml.ws.WebServiceClient;
import javax.xml.ws.WebServiceException;
import javax.xml.ws.WebServiceFeature;


/**
 * This class was generated by the JAX-WS RI.
 * JAX-WS RI 2.2.9-b130926.1035
 * Generated source version: 2.2
 * 
 */
@WebServiceClient(name = "UpperCaseServiceService", targetNamespace = "http://services/", wsdlLocation = "file:/home/snavi/Documents/distributed_systems/distributed_systems/client/src/UpperCaseService.wsdl")
public class UpperCaseServiceService
    extends Service
{

    private final static URL UPPERCASESERVICESERVICE_WSDL_LOCATION;
    private final static WebServiceException UPPERCASESERVICESERVICE_EXCEPTION;
    private final static QName UPPERCASESERVICESERVICE_QNAME = new QName("http://services/", "UpperCaseServiceService");

    static {
        URL url = null;
        WebServiceException e = null;
        try {
            url = new URL("file:/home/snavi/Documents/distributed_systems/distributed_systems/client/src/UpperCaseService.wsdl");
        } catch (MalformedURLException ex) {
            e = new WebServiceException(ex);
        }
        UPPERCASESERVICESERVICE_WSDL_LOCATION = url;
        UPPERCASESERVICESERVICE_EXCEPTION = e;
    }

    public UpperCaseServiceService() {
        super(__getWsdlLocation(), UPPERCASESERVICESERVICE_QNAME);
    }

    public UpperCaseServiceService(WebServiceFeature... features) {
        super(__getWsdlLocation(), UPPERCASESERVICESERVICE_QNAME, features);
    }

    public UpperCaseServiceService(URL wsdlLocation) {
        super(wsdlLocation, UPPERCASESERVICESERVICE_QNAME);
    }

    public UpperCaseServiceService(URL wsdlLocation, WebServiceFeature... features) {
        super(wsdlLocation, UPPERCASESERVICESERVICE_QNAME, features);
    }

    public UpperCaseServiceService(URL wsdlLocation, QName serviceName) {
        super(wsdlLocation, serviceName);
    }

    public UpperCaseServiceService(URL wsdlLocation, QName serviceName, WebServiceFeature... features) {
        super(wsdlLocation, serviceName, features);
    }

    /**
     * 
     * @return
     *     returns UpperCaseService
     */
    @WebEndpoint(name = "UpperCaseServicePort")
    public UpperCaseService getUpperCaseServicePort() {
        return super.getPort(new QName("http://services/", "UpperCaseServicePort"), UpperCaseService.class);
    }

    /**
     * 
     * @param features
     *     A list of {@link javax.xml.ws.WebServiceFeature} to configure on the proxy.  Supported features not in the <code>features</code> parameter will have their default values.
     * @return
     *     returns UpperCaseService
     */
    @WebEndpoint(name = "UpperCaseServicePort")
    public UpperCaseService getUpperCaseServicePort(WebServiceFeature... features) {
        return super.getPort(new QName("http://services/", "UpperCaseServicePort"), UpperCaseService.class, features);
    }

    private static URL __getWsdlLocation() {
        if (UPPERCASESERVICESERVICE_EXCEPTION!= null) {
            throw UPPERCASESERVICESERVICE_EXCEPTION;
        }
        return UPPERCASESERVICESERVICE_WSDL_LOCATION;
    }

}
