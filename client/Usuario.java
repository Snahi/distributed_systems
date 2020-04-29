public class Usuario {
	public String nombre;
	public String ip;
	public int port;

	public Usuario(String nombre, String ip, int port) {
		this.nombre = nombre;
		this.ip = ip;
		this.port = port;
	}

	public Usuario() {
		this.nombre="";
		this.ip="";
		this.port=-1;
	}

	public String getNombre() {
		return nombre;
	}

	public void setNombre(String nombre) {
		this.nombre = nombre;
	}

	public String getIp() {
		return ip;
	}

	public void setIp(String ip) {
		this.ip = ip;
	}

	public int getPort() {
		return port;
	}

	public void setPort(int port) {
		this.port = port;
	}

}