// Charles Vidal <cvidal AT ivsweb.com>
// Updated by David Bagley

import java.applet.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class xlockApplet extends Applet implements ItemListener {
	private static final long serialVersionUID = 42L;
	Button launchButton;
	Button launchInWinButton;
	Button quitButton;
	Choice choice;
	TextField textField;
	List list;
	int currentOption = 0;
	ErrorFrame errorFrame;

	// Array of option name
	String[] descriptOptions = {
		"Display",
		"Geometry",
		"Message Password",
		"Message Valid",
		"Message Invalid",
		"Font",
		"Program for mode",
		"File for mode",
		"Misc. Option"
	};

	String[] valueOptions = {"","","","","","","","",""};
	String[] cmdlineOptions = {
		"-display",
		"-geometry",
		"-password",
		"-validate",
		"-invalid",
		"-font",
		"-program",
		"-messagefile",
		""
	};
	String[] booleanOptions = {
		"-mono ",
		"-nolock ",
		"-remote ",
		"-allowroot ",
		"-enablesaver ",
		"-allowaccess ",
		"-grabmouse ",
		"-echokeys ",
		"-usefirst ",
		"-debug ",
		"-verbose ",
		"-inroot ",
		"-timeelapsed ",
		"-install ",
		"-wireframe ",
		"-showfps ",
		"-use3d "
	};
	Checkbox checkbox[] = new Checkbox[booleanOptions.length];

	public void init() {
		Panel Panel1 = new Panel();
		Panel Panel2 = new Panel();
		Panel Panel3 = new Panel();

		setLayout(new BorderLayout(10, 10));
		list = new List();
		choice = new Choice();
		for (int i = 0; i < descriptOptions.length; i++)
			choice.add(descriptOptions[i]);
		Panel3.add(choice);
		textField = new TextField(20);
		Panel3.add(textField);
		add("North", Panel3);
		add("Center", list);
		list.add("anemone");
		list.add("ant");
		list.add("ant3d");
		list.add("apollonian");
		list.add("atlantis");
		list.add("atunnels");
		list.add("ball");
		list.add("bat");
		list.add("bevel");
		list.add("blot");
		list.add("bouboule");
		list.add("bounce");
		list.add("boxed");
		list.add("braid");
		list.add("bubble");
		list.add("bubble3d");
		list.add("bug");
		list.add("cage");
		list.add("clock");
		list.add("coral");
		list.add("crystal");
		list.add("daisy");
		list.add("dclock");
		list.add("decay");
		list.add("deco");
		list.add("demon");
		list.add("dilemma");
		list.add("dino");
		list.add("discrete");
		list.add("dragon");
		list.add("drift");
		list.add("euler2d");
		list.add("eyes");
		list.add("fadeplot");
		list.add("fiberlamp");
		list.add("fire");
		list.add("flag");
		list.add("flame");
		list.add("flow");
		list.add("forest");
		list.add("fzort");
		list.add("galaxy");
		list.add("gears");
		list.add("glplanet");
		list.add("goop");
		list.add("grav");
		list.add("helix");
		list.add("hop");
		list.add("hyper");
		list.add("ico");
		list.add("ifs");
		list.add("image");
		list.add("invert");
		list.add("juggle");
		list.add("juggler3d");
		list.add("julia");
		list.add("kaleid");
		list.add("kumppa");
		list.add("lament");
		list.add("laser");
		list.add("life");
		list.add("life1d");
		list.add("life3d");
		list.add("lightning");
		list.add("lisa");
		list.add("lissie");
		list.add("loop");
		list.add("lyapunov");
		list.add("mandelbrot");
		list.add("marquee");
		list.add("matrix");
		list.add("maze");
		list.add("maze3d");
		list.add("mball");
		list.add("moebius");
		list.add("molecule");
		list.add("morph3d");
		list.add("mountain");
		list.add("munch");
		list.add("noof");
		list.add("nose");
		list.add("oct");
		list.add("pacman");
		list.add("paterson");
		list.add("penrose");
		list.add("petal");
		list.add("petri");
		list.add("pipes");
		list.add("polyominoes");
		list.add("puzzle");
		list.add("pyraminx");
		list.add("pyro");
		list.add("pyro2");
		list.add("qix");
		list.add("rain");
		list.add("roll");
		list.add("rotor");
		list.add("rubik");
		list.add("sballs");
		list.add("scooter");
		list.add("shape");
		list.add("sierpinski");
		list.add("sierpinski3d");
		list.add("skewb");
		list.add("slip");
		list.add("solitaire");
		list.add("space");
		list.add("sphere");
		list.add("spiral");
		list.add("spirolateral");
		list.add("spline");
		list.add("sproingies");
		list.add("stairs");
		list.add("star");
		list.add("starfish");
		list.add("strange");
		list.add("superquadrics");
		list.add("swarm");
		list.add("swirl");
		list.add("t3d");
		list.add("tetris");
		list.add("text3d");
		list.add("text3d2");
		list.add("thornbird");
		list.add("tik_tak");
		list.add("toneclock");
		list.add("triangle");
		list.add("tube");
		list.add("turtle");
		list.add("vines");
		list.add("voters");
		list.add("wator");
		list.add("wire");
		list.add("world");
		list.add("worm");
		list.add("xcl");
		list.add("xjack");
		list.add("blank");
		list.add("bomb");
		list.add("random");
		list.select(0);
		add("East", Panel1);
		Panel1.setLayout(new GridLayout(booleanOptions.length,  1));
		for (int i = 0; i < booleanOptions.length; i++) {
			checkbox[i] = new Checkbox(booleanOptions[i],
				null, false);
			Panel1.add(checkbox[i]);
		}
		add("South", Panel2);
		launchButton = new Button("Launch");
		launchInWinButton = new Button("Launch in Window");
		quitButton = new Button("Quit");
		Panel2.add(launchButton);
		Panel2.add(launchInWinButton);
		Panel2.add(quitButton);
		errorFrame = new ErrorFrame("An error occurred, can not launch xlock");
		errorFrame.setSize(350, 150);
		choice.addItemListener(this);

		launchButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event) {
				launch("xlock ");
			}
		});

		launchInWinButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event) {
				launch("xlock -inwindow ");
			}
		});

		textField.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event) {
				valueOptions[currentOption] =
					textField.getText();
				System.out.println(valueOptions[currentOption]);
			}
		});

		quitButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event) {
				System.exit(0);
			}
		});
	}

	public void itemStateChanged(ItemEvent event) {
		String label = choice.getSelectedItem();

		valueOptions[currentOption] = textField.getText();
		for (int i = 0; i < descriptOptions.length; i++) {
			if (descriptOptions[i].equals(label)) {
				textField.setText(valueOptions[i]);
				currentOption = i;
			}
		}
	}

	public String getBooleanOption() {
		String result = "";
		for (int i = 0; i < booleanOptions.length; i++) {
			if (checkbox[i].getState())
				result = result.concat(booleanOptions[i]);
		}
		return (result);
	}

	private void launch(String cmd) {
		String cmdline = cmd;

		for (int i = 0; i < descriptOptions.length; i++) {
			if (!valueOptions[i].equals("")) {
				cmdline =
					cmdline.concat(cmdlineOptions[i] + " " +
					valueOptions[i] + " ");
			}
		}
		cmdline = cmdline.concat(getBooleanOption());
		cmdline = cmdline.concat("-mode ");
		cmdline = cmdline.concat(list.getSelectedItem());
		try {
			System.out.println(cmdline);
			Runtime.getRuntime().exec(cmdline);
		} catch (Exception e) {
			errorFrame.setVisible(true);
		}
	}

	public xlockApplet() {
	}

	public static void main(String args[]) {
		Frame frame = new Frame("xlock");
		xlockApplet applet = new xlockApplet();
		WindowListener listener = new WindowAdapter() {
			public void windowClosing(WindowEvent event) {
				System.exit(0);
			}
		};
		frame.addWindowListener(listener);
		applet.init();
		frame.add("Center", applet);
		frame.setSize(350, 400);
		frame.setVisible(true);
	}
}

class ErrorFrame extends JFrame implements ActionListener {
	private static final long serialVersionUID = 42L;
	Label label;
	Button button;
	ErrorFrame(String error) {
		setLayout(new BorderLayout());
		label = new Label(error, Label.CENTER);
		add("Center", label);
		button = new Button("OK");
		add("South", button);
		setTitle(error);
		setCursor(new Cursor(Cursor.HAND_CURSOR));
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent event) {
				setVisible(false);
			}
		});
		button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event) {
				setVisible(false);
			}
		});
	}
	public void actionPerformed(ActionEvent event) {
			setVisible(false);
	}
}
