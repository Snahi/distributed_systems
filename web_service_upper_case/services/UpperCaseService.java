package services;

import javax.jws.WebService;
import javax.jws.WebMethod;

@WebService
public class UpperCaseService {
    // CONST ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    public static final int PORT = 7778;
    public static final String URL_FORMAT = "http://localhost:%d/upper_case";



    public static String getUrl()
    {
        return String.format(URL_FORMAT, PORT);
    }



    @WebMethod
    public String upperCase(String str)
    {
        str = str == null ? "" : str;
        return str.toUpperCase();
    }
}
